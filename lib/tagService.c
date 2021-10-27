#include "../include/initStruct.h"
#include "../include/utils.h"

MODULE_LICENSE("GPL");

tag_t *tagServiceArray[MAX_N_TAGS];
int global_nextId = 0;
int global_numTag = 0;

static spinlock_t tagLock;
rwlock_t tagLocks[MAX_N_TAGS];

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
        spin_lock_init(&levelLocks[i]);
        
        
        
        init_waitqueue_head(&(levelsArray[i]->waitingThreads));
        

        //printk("levelsArray[%d]= %d    num=%d\n",i,levelsArray[i],levelsArray[i]->num);
        //printk("levelsArray[%d]= %d    waitingThreads=%d\n",i,levelsArray[i],levelsArray[i]->waitingThreads);

    }

    printk("levelsArray= %d\n",levelsArray);    //ind array
    //printk("*levelsArray= %d\n",*levelsArray);  //primo elemen
    //printk("levelsArray[0]= %d\n",levelsArray[0]);  //primo elemen
    //printk("levelsArray[0]->num= %d\n",levelsArray[0]->num);  //primo elemen
    //printk("&levelsArray= %d\n",&levelsArray);  //ind array
    //printk("**levelsArray= %d\n",**levelsArray);  //ind array
    //return *levelsArray;
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

    //TODO: check permessi 
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
            wake_up_all(&(currLevel->waitingThreads));
            spin_unlock(&currTag->levelLocks[i]);
        }
        
    }
    
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

    printk("---waitForMessage: level=%d\n",level);
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

    printk("dentro waitForMessage: recuperato tag con ID %d a indirizzo %d\n",tag,currTag);

    level_t** tagLevels= currTag->levels;

    //ora metto thread in attesa di un msg 

    //+1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, +1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), +1);

    resWaitEvent = wait_event_interruptible(tagLevels[myLevel]->waitingThreads,tagLevels[myLevel]->msg!=NULL);

    printk("\n---dentro waitForMessage: DOPO WAIT_EVENT  -  %d thread in tag  -  %d thread in wq\n",currTag->numThreads, tagLevels[myLevel]->numThreadsWq);

    if (resWaitEvent==0){
        
        
        //TODO: if BUFFER==NULL -> doppia condizione con awake all anche
        printk("dentro waitForMessage: receiver thread woke up because condition was TRUE");
        
    }
    else {
        //(res == -ERESTARTSYS)

        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
        __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);
        printk("dentro waitForMessage: receiver thread woke up by a signal");
        return -1;

    }

    //recupero msg (cioè cosa faccio dopo che thread è stato svegliato)
    // da field del livello
    // TODO: size = tagLevels[myLevel]->lastSize     min(lastSize,size)

    int resultCopy = __copy_to_user(buffer,tagLevels[myLevel]->msg, min(tagLevels[myLevel]->lastSize,size));

    printk("\ndentro waitForMessage: resultCopy = %d \n",resultCopy);
    //-1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, -1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);

    //printk("\ndentro waitForMessage: resultCopy = %d \n",resultCopy);

    //printk("dentro waitForMessage: tagLevels[myLevel]->msg: %s\n",tagLevels[myLevel]->msg);
    printk("dentro waitForMessage: thr %d - prima copy_to_user MESSAGGIO LETTO (rcv) da thread è: %s\n",uid.val, buffer);

    //strcpy(tagLevels[myLevel]->msg,buffer);

    printk("\ndentro waitForMessage: PRIMA IF livello%d->numThreadsWq=%d\n",myLevel,tagLevels[myLevel]->numThreadsWq);

    write_lock(&tagLocks[i]);
    if(tagLevels[myLevel]->numThreadsWq==0){

        //printk("\ndentro waitForMessage: NELL'IF livello%d->numThreadsWq=%d\n",myLevel,tagLevels[myLevel]->numThreadsWq);

        spin_lock(&(currTag->levelLocks[myLevel]));
        
        printk("\ndentro waitForMessage: NELL'IF - PRIMA NULL - msg=%s\n",tagLevels[myLevel]->msg);
        //TODO: deallocare il buffer prima di metterlo a null
        tagLevels[myLevel]->msg=NULL;
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

/*
todo: per array di lock posso fare un array dove nella struct (o dei tag)
o dell'array di lock mantengo posizione di dove sta il tag oppure proprio
un puntatore al tag stesso

faccio array di spinlock e nella struct del tag mantengo indice  

metti come id posizione in tagArray
fai un array di 256 spinlock e 256 array da 32 spinlock

perche mi serve lock del singolo tag: ad esempio se ho 
    -due send su stesso livello: devo mettere lock su stesso livello
    -lettura di numThreads nel tag: mi serve lock sul singolo tag
    -remove del tag: mi serve lock su singolo tag
*/
 /*
int deleteLevels(tag_t tag){

   
    int i;

    level_t **levels = tag->levels;

    for(i=0; i<N_LEVELS; i++){

        spin_lock(&(tag->levelLocks[i]));
        //TODO: dealloco buffer del livello,
        //nella struct livello metto puntatore al buffer = null
        //       ==      struct livello
        //         =     puntatore nell array dei livelli a null
        //dealloco tutto l'array levelsArray (???)
        //dealloco tag service
        //metto a nulla puntatore a tag service in tagServiceArray



        kfree(levels[i]);
        
        spin_unlock(&(tag->levelLocks[i]));
        
    }
    


}
*/

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

                //TODO: fare free dei livelli
                read_unlock(&tagLocks[i]);

                write_lock(&tagLocks[i]);
                kfree(tagServiceArray[i]);
                tagServiceArray[i] = NULL;
                //kfree(tagServiceArray[i]);
                
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




/*
-se qui sto facendo CREATE, e c'è gia chiave uguale ritorno -1 (user
deve fare la OPEN con stessa chiave)
-se key = 0, istanzio tag e procedo. 
*/
int addTag(int key, kuid_t userId, pid_t creatorProcessId, int perm){

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
    newTag->creatorProc = creatorProcessId; //per private
    newTag->numThreads = 0;                 //n threads nelle 32 wq
    printk("dentro addTag: newTag->creatorUserId= %d\n",newTag->creatorUserId);
    printk("dentro addTag: newTag->creatorProc= %d\n",newTag->creatorProc);

    //inizializzazione livelli
    initLevels(levelsArray, newTag->levelLocks);
    
    printk("dentro addTag: levels= %d\n",levelsArray);
    //printk("dentro addTag:*levels= %d\n",*levels);

    newTag->levels = levelsArray;
    printk("dentro addTag: newTag->levels= %d\n",newTag->levels);
    //printk("dentro addTag: newTag->levels[5].num= %d\n",newTag->levels[5]->num);
    

    //inizializzazione id
    __sync_fetch_and_add(&global_nextId, +1);
    newTag->ID = global_nextId;
    printk("newTag->ID = %d\n", newTag->ID);
    printk("newTag = %d\n", newTag);

    printk("newTag:\n");
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
            //printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
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
            //se avviene remove mentre faccio send la send va a picco 
            currTag = tagServiceArray[i];
            break;
            //printk("dentro getTagFromID: tagServiceArray[i] = %d\n", tagServiceArray[i]);
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
    
    printk("dentro deliverMsg: recuperato tag con ID %d a indirizzo %d\n",tagId,currTag);
        
    //recupero livello d'interesse
    level_t** tagLevels= currTag->levels;
    level_t* currLevel = tagLevels[level-1];

    spin_lock(&currTag->levelLocks[level-1]);

    if(currLevel->msg != NULL){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR "dentro deliverMsg: è gia in uso il tag con ID %d al livello %d questa send viene scartata\n",tagId, level-1);
        return -1;
    }

    currLevel->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);

    currLevel->lastSize = size;
    
    //copy from user
    int copiati = __copy_from_user(currLevel->msg,msg,size);
    printk(KERN_INFO "Dentro deliverMsg: copiati = %d\n", copiati);

    if (copiati!=0){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR "ERRORE dentro deliverMsg: copy_from_user andata male\n");
        return -1;
    }

    printk("dentro deliverMsg: ID = %d  currLevel->msg = %s\n",currTag->ID,currLevel->msg);

    //=====wait queue=====
    //printk("dentro deliverMsg: ID = %d  currLevel->waitingThreads = %d\n",currTag->ID,currLevel->waitingThreads);
    //qui bisogna svegliare i threads
    wake_up_all(&(currLevel->waitingThreads));
    
    spin_unlock(&currTag->levelLocks[level-1]);
    read_unlock(&tagLocks[i]);


    //TODO: kfree dopo che thr di quel liv non ci sono piu


    return 0;
    


}

