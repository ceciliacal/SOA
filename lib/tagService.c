#include "../include/tagService.h"
#include "../include/utils.h"

MODULE_LICENSE("GPL");

tag_t *tagServiceArray[MAX_N_TAGS];
int global_nextId = 0;
int global_numTag = 0;

rwlock_t tagLocks[MAX_N_TAGS];

tag_t** getTagServiceArray(void){
    return tagServiceArray;
}

rwlock_t* getTagLocks(void){
    return tagLocks;
}

void initTagLocks(void){
    int i;
    for (i=0;i<MAX_N_TAGS;i++){
        rwlock_init(&tagLocks[i]);
    }
}


void initLevels(level_t* levelsArray[N_LEVELS],spinlock_t levelLocks[N_LEVELS]){

    int i;

    for (i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->numThreadsWq=0;
        levelsArray[i]->number=i+1;
        levelsArray[i]->wakeUpCondition=0;
        spin_lock_init(&levelLocks[i]);
          
        init_waitqueue_head(&(levelsArray[i]->waitingThreads));


    }

}

/*
pid non lo considero perché non posso richiamare tag_ctl 2 volte, quindi solo la
prima volta che creo il tag e quindi non ho la open. Il permission=1 solo se è CREATE
il command, perche nel testo dice che permission è usato per indicare per quale scopo
il tag è stato CREATO, quindi in open permission è sempre 0

*/
int openTag(int key, kuid_t currentUserId){

    int i;
    tag_t* tag;

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->key == key){
            
            tag = tagServiceArray[i];
            
            if (tag->permission == 1 && tag->creatorUserId.val!=currentUserId.val){
                printk("openTag: permission denied\n");
                read_unlock(&tagLocks[i]);
                return -1;  //non ho permesso perche utente è diverso 

            }
            if (tag->private==1){   //non posso fare open di tag 
                read_unlock(&tagLocks[i]);
                printk("openTag: tag cannot be opened since it's private\n");
                return -1;
            }

            printk("openTag: returning tag id = %d\n",tag->ID);
            read_unlock(&tagLocks[i]);
            return tag->ID;

        }
        read_unlock(&tagLocks[i]);

    }

    return -1;   //not found

}


/*
    uid e pid in input vengono presi comunque lato kernel in TAG_GET usando kuid_t
    l'utente che sta eseguendo thread e invoca tag get può essere cambiato modificando
    l'euid facendo setuid. per testare

    gestione concorrenza con lock: quando 2 thread differenti cercano di fare op
    su stesso tag

    devo fare pure controllo della chiave: se voglio aggiungere tag e metto key 2
    e gia c'è un tag con key 2 allora non ne creo uno nuovo ma restituisco quello che 
    gia c'è (o restituisco -1 se il command è create)

*/

int checkAwakeAll(int tag, kuid_t currentUserId){

    int i;
    tag_t* currTag = NULL; //get tag from ID
    level_t* currLevel;

    printk("in AWAKE_ALL\n");
    for(i=0;i<MAX_N_TAGS;i++){
        
        read_lock(&tagLocks[i]);

        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tag){
            
            currTag = tagServiceArray[i];
            break;
            //printk("dentro getTagFromID: tagServiceArray[i] = %d\n", tagServiceArray[i]);
        }
        read_unlock(&tagLocks[i]);
    }

    if (currTag==NULL){
        printk("dentro checkAwakeAll: ERRORE, tag con ID %d not found\n",tag);
        read_unlock(&tagLocks[i]);
        return -1;
    }

    if (checkCorrectCondition(currTag, currentUserId) == -1){
        read_unlock(&tagLocks[i]);
        return -1;
    }

    //recupero livelli
    level_t** tagLevels= currTag->levels;
    
    //write lock su tag
    for (i=0;i<N_LEVELS;i++){

        //per ogni livello, prendo lock su quel livello e sveglio 
        //tutti i thread che sono in quella wq

        if (tagLevels[i]!=NULL){

            spin_lock(&currTag->levelLocks[i]);

            level_t* currLevel = tagLevels[i];
            currLevel->wakeUpCondition=1;
            wake_up_all(&(currLevel->waitingThreads));


            spin_unlock(&currTag->levelLocks[i]);
        }
        
    }


    printk("DOPO CICLO IN AWAKE: currTag=%d  currLevel=%d\n",currTag,currLevel);
    printk("DOPO CICLO IN AWAKE: threads in TAG=%d\n",currTag->numThreads);
    
    read_unlock(&tagLocks[i]);
    return 0;
}

void printArray(void){
    int i;
    printk("\n\n");
    for(i=0;i<MAX_N_TAGS;i++){
        
        if (tagServiceArray[i]!=NULL){
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
        }
    }
    printk("\n\n");
}

int waitForMessage(int tag,int level, char* buffer, size_t size, kuid_t uid){
    
    int resWaitEvent;
    int i;
    tag_t* currTag = NULL; //get tag from ID

    //printk("---waitForMessage: level=%d\n",level);
    if (level<1 || level>32){
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    int myLevel = level-1;

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tag){
            
            currTag = tagServiceArray[i];
            read_unlock(&tagLocks[i]);
            break;
        }

    }

    if (currTag==NULL){
        read_unlock(&tagLocks[i]);
        printk("dentro waitForMessage: ERRORE, tag con ID %d not found\n",tag);
        return -1;
    }

    if(checkCorrectCondition(currTag, uid)==-1){
        read_unlock(&tagLocks[i]);
        return -1;
    }

    //printk("dentro waitForMessage: recuperato tag con ID %d a indirizzo %d\n",tag,currTag);

    level_t** tagLevels= currTag->levels;

    //ora metto thread in attesa di un msg 
    //+1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, +1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), +1);

    resWaitEvent = wait_event_interruptible(tagLevels[myLevel]->waitingThreads,tagLevels[myLevel]->wakeUpCondition==1);

    printk("\n---dentro waitForMessage: DOPO WAIT_EVENT  -  %d thread in tag  -  %d thread in wq\n",currTag->numThreads, tagLevels[myLevel]->numThreadsWq);

    if (tagLevels[myLevel]->msg!=NULL&&tagLevels[myLevel]->wakeUpCondition==1){
        
        printk("dentro waitForMessage: receiver thread woke up because of SEND\n");
        
    }
    else if (tagLevels[myLevel]->msg==NULL&&tagLevels[myLevel]->wakeUpCondition==1){

        printk("dentro waitForMessage: receiver thread woke up because of AWAKE_ALL\n");
        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
        __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);
        return -1;

    }
    else {
    
        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
        __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);
        printk("dentro waitForMessage: receiver thread woke up by a signal");
        return -1;

    }


    int resultCopy = __copy_to_user(buffer,tagLevels[myLevel]->msg, min(tagLevels[myLevel]->lastSize,size));

    //printk("\ndentro waitForMessage: resultCopy = %d \n",resultCopy);
    //-1 thread in sleep
    printk("dentro waitForMessage: thr %d - prima copy_to_user MESSAGGIO LETTO (rcv) da thread è: %s\n",tagLevels[myLevel]->numThreadsWq, buffer);
    __sync_fetch_and_add(&currTag->numThreads, -1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);

    //printk("\ndentro waitForMessage: PRIMA IF livello%d->numThreadsWq=%d\n",myLevel,tagLevels[myLevel]->numThreadsWq);

    //last receiver
    write_lock(&tagLocks[i]);
    if(tagLevels[myLevel]->numThreadsWq==0){

        spin_lock(&(currTag->levelLocks[myLevel]));
        
        printk("\ndentro waitForMessage: NELL'IF - PRIMA NULL - msg=%s\n",tagLevels[myLevel]->msg);
        kfree(tagLevels[myLevel]->msg);
        tagLevels[myLevel]->msg=NULL;
        tagLevels[myLevel]->wakeUpCondition=0;

        printk("\ndentro waitForMessage: NELL'IF - DOPO NULL - msg=%s\n",tagLevels[myLevel]->msg);

        spin_unlock(&(currTag->levelLocks[myLevel]));

    }

    if(tagLevels[myLevel]->numThreadsWq<0){

        printk("\n\n ==== dentro waitForMessage: numThreadsWq = %d    -    MINORE DI 0!!! ==== \n\n",tagLevels[myLevel]->numThreadsWq);

    }

    write_unlock(&tagLocks[i]);
    printk("dentro waitForMessage: messaggio ricevuto dal thread %d è: %s\n",uid.val, buffer);
    
    return 0;
    

}


int removeTag(int tag, kuid_t currentUserId){

    int i;
    
    if (global_numTag<1){
        printk("Errore in removeTag: non ci sono tag da rimuovere, inserirne almeno uno prima.\n");
        return -1;
    }

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID==tag){

            if (checkCorrectCondition(tagServiceArray[i], currentUserId) == -1){
                read_unlock(&tagLocks[i]);
                return -1;
            }         

            //check se ci sono thread nella wq
            if (tagServiceArray[i]->numThreads==0){

                read_unlock(&tagLocks[i]);

                write_lock(&tagLocks[i]);

                kfree(tagServiceArray[i]->levels);
                tagServiceArray[i]->levels = NULL;
                kfree(tagServiceArray[i]);
                tagServiceArray[i] = NULL;
                
                write_unlock(&tagLocks[i]);
                __sync_fetch_and_add(&global_numTag, -1);
                
                
                
                return 0;

            }
            
            else{

                read_unlock(&tagLocks[i]);
                printk("RemoveTag: ERRORE ci sono %d thread nella wait queue\n",tagServiceArray[i]->numThreads);
                return -1;
            }
            
        
        }
        read_unlock(&tagLocks[i]);

    }

    //vuol dire che tag non c'era
    printk("RemoveTag: ERRORE tag %d non è presente nel sistema\n",tag);
    return -1;
}



//TODO: togliere perm
int addTag(int key, kuid_t userId, int perm){

    int i;
    tag_t* newTag;

    if (global_numTag>MAX_N_TAGS){
        printk("Errore in addTag: il numero dei tag presenti nel servizio ha raggiunto il limite di 256.\n");
        return -1;
    }

    if (key!=0){

        for(i=0;i<MAX_N_TAGS;i++){

            read_lock(&tagLocks[i]);
        
            if (tagServiceArray[i]!=NULL&&tagServiceArray[i]->key==key){
                printk("addTag: there is already a tag with key %d\n. Try again with OPEN command.\n",tagServiceArray[i]->key); 
                
                read_unlock(&tagLocks[i]);
                return -1;
            }
            read_unlock(&tagLocks[i]);
        }
    }

    
    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);
    level_t** levelsArray = (level_t**) kzalloc(sizeof(level_t*)*N_LEVELS,GFP_KERNEL);

    if (perm==1){
        newTag->permission = 1;
    } 
    else {
        newTag->permission = 0;
    }
    
    newTag->key = key;
    newTag->creatorUserId = userId;         //per permission
    newTag->numThreads = 0;                 //n threads nelle 32 wq
    
    //inizializzazione livelli
    initLevels(levelsArray, newTag->levelLocks);
    newTag->levels = levelsArray;

    //inizializzazione id
    __sync_fetch_and_add(&global_nextId, +1);
    newTag->ID = global_nextId;

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);

        /*
        if (tagServiceArray[i]!=NULL){
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
        }
        */
        if (tagServiceArray[i]==NULL){

            read_unlock(&tagLocks[i]);
            write_lock(&tagLocks[i]);
            tagServiceArray[i] = newTag;
            write_unlock(&tagLocks[i]);
            __sync_fetch_and_add(&global_numTag, +1);
            break;
        }
        read_unlock(&tagLocks[i]);

    }
    
    return newTag->ID;
    
}



int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId){
    
    int i;
    tag_t* currTag = NULL; //get tag from ID

    

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tagId){  
            currTag = tagServiceArray[i];
            
            break;
        }
        read_unlock(&tagLocks[i]);

    }

    

    if (currTag==NULL){
        printk("dentro deliverMessage: ERRORE, tag con ID %d not found\n",tagId);
        read_unlock(&tagLocks[i]);
        return -1;
    }


    if (checkCorrectCondition(currTag, currentUserId) == -1){
        read_unlock(&tagLocks[i]);
        return -1;
    }
    
    //printk("dentro deliverMsg: recuperato tag con ID %d a indirizzo %d\n",tagId,currTag);
        
    //recupero livello d'interesse
    level_t** tagLevels= currTag->levels;
    level_t* currLevel = tagLevels[level-1];

    spin_lock(&currTag->levelLocks[level-1]);

    if(currLevel->msg != NULL){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR "dentro deliverMsg: è gia in uso il tag con ID %d al livello %d questa send viene scartata\n",tagId, level);
        return -1;
    }

    currLevel->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);

    currLevel->lastSize = size;
    
    //copy from user
    int copiati = __copy_from_user(currLevel->msg,msg,size);
    //printk(KERN_INFO "Dentro deliverMsg: copiati = %d\n", copiati);

    if (copiati!=0){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR "ERRORE dentro deliverMsg: copy_from_user andata male\n");
        return -1;
    }

    printk("dentro deliverMsg: ID = %d  currLevel->msg = %s\n",currTag->ID,currLevel->msg);

    //=====wait queue=====
    //qui bisogna svegliare i threads
    currLevel->wakeUpCondition=1;
    wake_up_all(&(currLevel->waitingThreads));
    
    spin_unlock(&currTag->levelLocks[level-1]);
    read_unlock(&tagLocks[i]);
    
    return 0;

}

