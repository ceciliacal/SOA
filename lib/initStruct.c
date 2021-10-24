#include "../include/initStruct.h"
#include "../include/tagIdGeneration.h"
#include "../include/utils.h"

MODULE_LICENSE("GPL");

//qui faccio inizializzazione del sistema
//creo array per i tag , inizializzo livelli
//aggiungo tag all'array, lo rimuovo
//faccio istantiate del tag get

tag_t *tagServiceArray[MAX_N_TAGS];
int global_nextId = 0;
int global_numTag = 0;

static spinlock_t tagLock;

void initLevels(level_t* levelsArray[N_LEVELS],spinlock_t levelLocks[N_LEVELS]){

    int i;

    for (i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->num=i+1;
        spin_lock_init(&levelLocks[i]);
        
        
        wait_queue_head_t* myQueue;
        myQueue = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
        init_waitqueue_head(myQueue);
        levelsArray[i]->waitingThreads = myQueue;

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

    //todo: check permessi 

    //va messo LOCK prima del for perché faccio subito check della chiave
    spin_lock(&tagLock);
    for(i=0;i<MAX_N_TAGS;i++){

        
        if (tagServiceArray[i]->key == key){

            tag = tagServiceArray[i];
            
            if (tag->permission == 1 && tag->creatorUserId.val!=currentUserId.val){
                printk("openTag: permission denied\n");
                spin_unlock(&tagLock);
                return -1;  //non ho permesso perche utente è diverso 

            }
            if (tag->private==1){   //non posso fare open di tag 
                spin_unlock(&tagLock);
                printk("openTag: tag cannot be opened since it's private\n");
                return -1;
            }

            printk("openTag: returning tag id = %d\n",tag->ID);
            spin_unlock(&tagLock);
            return tag->ID;

        }

    }
    spin_unlock(&tagLock);


    return 0;   //not found

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

int waitForMessage(int tag,int myLevel, char* buffer, size_t size, kuid_t uid){
    
    int res;
    tag_t* currTag; //get tag from ID
    int level = myLevel-1;

    currTag = getTagFromID(tag);
    if(checkCorrectCondition(currTag, uid)==-1){
        return -1;
    }

    printk("dentro waitForMessage: recuperato tag con ID %d a indirizzo %d\n",tag,currTag);

    level_t** tagLevels= currTag->levels;

    //ora metto thread in attesa di un msg 

    //+1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, +1);

    res = wait_event_interruptible(*tagLevels[level]->waitingThreads,tagLevels[level]->msg!=NULL);
    printk("dentro waitForMessage: risvegliati %d thread dalla wait queue!!!!!!!!!\n",currTag->numThreads);

    if (res == 0){
        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
    }

    if (res == -ERESTARTSYS){
        printk("dentro waitForMessage: receiver thread was interrupted by a signal");
        return -1;
    }

    //recupero msg (cioè cosa faccio dopo che thread è stato svegliato)
    // da field del livello
    strcpy(tagLevels[level]->msg,buffer);
    printk("dentro deliverMsg: messaggio ricevuto dal thread %d è: %s\n",uid.val, buffer);
    //copy to user

    //if thread si è svegliato per send o per segnale posix

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

int removeTag(int tag){

    //todo: mettere permessi per check utente

    int i;
    spin_lock(&tagLock);
    printk("\nin removeTag: \n");

    if (global_numTag<1){
        printk("Errore in removeTag: non ci sono tag da rimuovere, inserirne almeno uno prima.\n");
        return -1;
    }

    for(i=0;i<MAX_N_TAGS;i++){

        printk("tagServiceArray[%d]->ID = %d\n", i,tagServiceArray[i]->ID);
        
        if (tagServiceArray[i]->ID==tag){

            printk("sto nell if con ID = %d\n",tagServiceArray[i]->ID);
            
            //check se ci sono thread nella wq
            if (tagServiceArray[i]->numThreads==0){

                //todo: fare free dei livelli

                tagServiceArray[i] = NULL;
                kfree(tagServiceArray[i]);
                __sync_fetch_and_add(&global_numTag, -1);
                spin_unlock(&tagLock);
                return 0;

            }
            else{
                printk("RemoveTag: ERRORE ci sono %d thread nella wait queue\n",tagServiceArray[i]->numThreads);
            }
            
        
        }

    }

    //vuol dire che tag non c'era
    spin_unlock(&tagLock);
    return -1;
}

int addTag(int key, kuid_t userId, pid_t creatorProcessId, int perm){

    //controllo che non esiste già una chiave uguale (se key!=0)
    /*
    -se qui sto facendo CREATE, e c'è gia chiave uguale ritorno -1 (user
    deve fare la OPEN con stessa chiave)
    -se key = 0, istanzio tag e procedo. 

    */
    int i;

    if (global_numTag>MAX_N_TAGS){
        printk("Errore in addTag: il numero dei tag presenti nel servizio ha raggiunto il limite di 256.\n");
        return -1;
    }

    if (key!=0){

        //va messo LOCK -> uso spinlock a diff di rw perche quest ultimi 
        //vengono usati per strutture dati che hanno molte piu read che write
        //(e non è questo il caso)
        spin_lock(&tagLock);
        for(i=0;i<MAX_N_TAGS;i++){
        
            if (tagServiceArray[i]!=NULL&&tagServiceArray[i]->key==key){
                spin_unlock(&tagLock);
                printk("addTag: there is already a tag with key %d\n. Try again with OPEN command.\n",tagServiceArray[i]->key); 
                
                return -1;
            }
        }
        spin_unlock(&tagLock);
    }

    
    tag_t* newTag;
    level_t** levelsArray = (level_t**) kzalloc(sizeof(level_t*)*N_LEVELS,GFP_KERNEL);
    //spinlock_t** levelLocksArray = (spinlock_t**) kzalloc(sizeof(spinlock_t)*N_LEVELS,GFP_KERNEL);

    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);
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


    //aggiunto di newTag all'array
    //ANCHE qui va usato lock
    printk("newTag:\n");
    spin_lock(&tagLock);
    for(i=0;i<MAX_N_TAGS;i++){
        //printk("prima if: i=%d\n",i);
        if (tagServiceArray[i]!=NULL){
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
        }
        
        if (tagServiceArray[i]==NULL){
            tagServiceArray[i] = newTag;
            __sync_fetch_and_add(&global_numTag, +1);
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
            break;
        }

    }
    spin_unlock(&tagLock);
    
    return newTag->ID;
    
}


tag_t* getTagFromID(int id){

    int i;
    //printk("dentro getTagFromID: id = %d\n",id);
    for(i=0;i<MAX_N_TAGS;i++){
        
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == id){
            return tagServiceArray[i];
            //printk("dentro getTagFromID: tagServiceArray[i] = %d\n", tagServiceArray[i]);
        }
        

    }

    return NULL;
}


level_t* getLevel(tag_t* tag, int levelNumber){

    level_t** tagLevels= tag->levels;
    return tagLevels[levelNumber-1];
}

/*
vedi lock da mettere (send bloccante !!!)

*/
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId){
    
    tag_t* currTag; //get tag from ID

    //questo lo lascio qua cosi se livello sbagliato manco cerco tag
    if (level<1 || level>32){
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    //qui lock su tag non dovrebbe servire
    currTag = getTagFromID(tagId);

    if (checkCorrectCondition(currTag, currentUserId) == -1){
        return -1;
    }
   
    printk("dentro deliverMsg: recuperato tag con ID %d a indirizzo %d\n",tagId,currTag);
    
    level_t* currLevel = getLevel(currTag, level);
    currLevel->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);

    spin_lock(&currTag->levelLocks[level-1]);
    
    //copy from user
    strcpy(currLevel->msg,msg);
    printk("dentro deliverMsg: currLevel->msg = %s\n",currLevel->msg);

    //=====wait queue=====
    printk("dentro deliverMsg: currLevel->waitingThreads = %d\n",currLevel->waitingThreads);
    //qui bisogna svegliare i threads
    wake_up_interruptible(currLevel->waitingThreads);
    
    spin_unlock(&currTag->levelLocks[level-1]);

    return 0;

}

/*
se key sono uguali di 2 thread diversi (e key!=0) allora restituisco stesso tag
mentre se key=0 i tag devono essere diversi. e se faccio ipc private NON POSSO FARE OPEN!
quindi creo e basta il tag con key=0 e id generato randomicamente
*/


/*

int init_module(void){
    
    initRandIdArray();
    level_t* levels[N_LEVELS];
    
    addTag(1,0,0,1);
    addTag(1,0,0,1);
    //addTag(2,0,0,0);
    
    //int idtag= openTag(1,0);
    //int idtag1= openTag(2,1);
    //int idtag2= openTag(1,1);
   
    //printk("idtag=%d",idtag,"idtag1=%d",idtag1,"idtag2=%d\n",idtag2);
     
    return 0;
}



void cleanup_module(void){
    printk("CLEANUP!!\n");
    
}
*/


