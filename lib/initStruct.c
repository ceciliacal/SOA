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

static spinlock_t tagLock;

void initLevels(level_t* levelsArray[N_LEVELS]){

    int i;

    for (i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->num=i+1;
        levelsArray[i]->numThreads = 0;
        
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

    //va messo LOCK prima del for perché faccio subito check della chiave
    spin_lock(&tagLock);
    for(i=0;i<MAX_N_TAGS;i++){

        //do something. check della chiave scorrendo array di tag
        
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

int removeTag(int tag){

    int i;
    spin_lock(&tagLock);
    printk("\nin removeTag: \n");
    for(i=0;i<MAX_N_TAGS;i++){
        printk("tagServiceArray[%d]->ID = %d\n", i,tagServiceArray[i]->ID);
        if (tagServiceArray[i]->ID==tag){
            printk("sto nell if con ID = %d\n",tagServiceArray[i]->ID);   
            tagServiceArray[i] = NULL;
            kfree(tagServiceArray[i]);
            spin_unlock(&tagLock);
            return 0;
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
    printk("dentro addTag: newTag->creatorUserId= %d\n",newTag->creatorUserId);
    printk("dentro addTag: newTag->creatorProc= %d\n",newTag->creatorProc);

    //inizializzazione livelli
    /*
    initLevels(levels);
    levelsArray = levels;
    */

    initLevels(levelsArray);
    
    printk("dentro addTag: levels= %d\n",levelsArray);
    //printk("dentro addTag:*levels= %d\n",*levels);

    newTag->levels = levelsArray;
    printk("dentro addTag: newTag->levels= %d\n",newTag->levels);
    //printk("dentro addTag: newTag->levels[5].num= %d\n",newTag->levels[5]->num);
    

    //inizializzazione id
    /*
    TODO: io qui dovrei mettere prima lock su randId poi metterlo su
    array di tag e poi sbloccarli entrambi (prima array poi rand) una volta
    aggiunto il tag

    */
    //int id = generateId();
    //newTag->ID = id;


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
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
            break;
        }

    }
    spin_unlock(&tagLock);
    
    return newTag->ID;
    
}

/*
TODO: mantenere var globale che incrementa numero di tag in modo
che non vada oltre MAX_N_TAGS e il for lo posso fare usando quella


*/

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
todo: ragionare sul fatto di tenere struttura dati separata tra info dei tag
e i tag veri e propri. Cioè se metto array che contiene le info con gli ID randomici
che ho gia creato e poi la riaccedo tipo per controllarla nella tag send/rcv/ctl ???
però non dovrei mettere un lock che comunque fa si che mentre controllo l'array con le info
questo non venga aggiornato???
probabilmente sì a questo punto il vantaggio non ce l'ho

vedi lock da mettere (send bloccante !!!)

*/
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId){
    
    tag_t* currTag; //get tag from ID
    int preCheck;

    //questo lo lascio qua cosi se livello sbagliato manco cerco tag
    if (level<1 || level>32){
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    currTag = getTagFromID(tagId);
    
    preCheck = checkCorrectCondition(currTag, currentUserId);

    if (preCheck == -1){
        return -1;
    }
   
    printk("dentro deliverMsg: recuperato tag con ID %d a indirizzo %d\n",tagId,currTag);

/*
    level_t** tagLevels= currTag->levels;
    printk("dentro deliverMsg: tagLevels= %d\n",tagLevels);
    printk("dentro deliverMsg: tagLevels[0]->num = %d\n", tagLevels[0]->num);
    printk("dentro deliverMsg: tagLevels[0]->msg = %s\n", tagLevels[0]->msg);

    tagLevels[level-1]->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);
    printk("dentro deliverMsg: tagLevels[level-1]->msg = %s\n", tagLevels[level-1]->msg);
    printk("dentro deliverMsg: msg= %s\n", msg);

    strcpy(tagLevels[level-1]->msg,msg);
    printk("dentro deliverMsg: tagLevels[level-1]->msg = %s\n",tagLevels[level-1]->msg);
*/

    level_t* currLevel = getLevel(currTag, level);
    currLevel->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);
    strcpy(currLevel->msg,msg);
    printk("dentro deliverMsg: currLevel->msg = %s\n",currLevel->msg);



    //=====wait queue=====
    printk("dentro deliverMsg: currLevel->waitingThreads = %d\n",currLevel->waitingThreads);

    //qui bisogna svegliare i threads
    wake_up_interruptible(currLevel->waitingThreads);

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


