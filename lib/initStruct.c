#include "../include/initStruct.h"

MODULE_LICENSE("GPL");

//qui faccio inizializzazione del sistema
//creo array per i tag , inizializzo livelli
//aggiungo tag all'array, lo rimuovo
//faccio istantiate del tag get

tag_t *tagServiceArray[MAX_N_TAGS];
int randId[MAX_N_TAGS];

static spinlock_t tagLock;


void addElemToLevel(void){

    level_t** tagLevels1 = tagServiceArray[0]->levels;
    printk("\n\ntag 1 = %d\n",tagServiceArray[0]->ID);
    printk("tagLevels1 = %d\n",tagLevels1);
    printk("*tagLevels1= %d\n",*tagLevels1);
    printk("tagLevels1[0]= %d\n",tagLevels1[0]);
    printk("**tagLevels1= %d\n",**tagLevels1);
    //printk("tagLevels1[0]->num= %d\n",tagLevels1[0]->num);

    level_t** tagLevels2 = tagServiceArray[1]->levels;
    printk("\n\ntag 2 = %d\n",tagServiceArray[1]->ID);
    printk("tagLevels2 = %d\n",tagLevels2);
    printk("*tagLevels2= %d\n",*tagLevels2);
    printk("tagLevels2[0]= %d\n",tagLevels2[0]);
    //printk("tagLevels2[0]->num= %d\n",tagLevels2[0]->num);

    level_t** tagLevels3 = tagServiceArray[2]->levels;
    printk("\n\ntag 3 = %d\n",tagServiceArray[2]->ID);
    printk("tagLevels3 = %d\n",tagLevels3);
    printk("*tagLevels3= %d\n",*tagLevels3);
    printk("tagLevels3[0]= %d\n",tagLevels3[0]);
    //printk("tagLevels3[0]->num= %d\n",tagLevels3[0]->num);



}


void initLevels(level_t* levelsArray[N_LEVELS]){

    int i;

    for (i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->num=i+1;
        
        wait_queue_head_t* myQueue;
        myQueue = kzalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
        init_waitqueue_head(myQueue);
        levelsArray[i]->waitingThreads = myQueue;

        printk("levelsArray[%d]= %d    num=%d\n",i,levelsArray[i],levelsArray[i]->num);
        printk("levelsArray[%d]= %d    waitingThreads=%d\n",i,levelsArray[i],levelsArray[i]->waitingThreads);

    }

    printk("levelsArray= %d\n",levelsArray);    //ind array
    printk("*levelsArray= %d\n",*levelsArray);  //primo elemen
    printk("levelsArray[0]= %d\n",levelsArray[0]);  //primo elemen
    printk("levelsArray[0]->num= %d\n",levelsArray[0]->num);  //primo elemen
    printk("&levelsArray= %d\n",&levelsArray);  //ind array
    printk("**levelsArray= %d\n",**levelsArray);  //ind array
    //return *levelsArray;
}

//qua va messo lock perche sto controllando che non ci sia stesso numero randomico
//all'interno dell'array randId perché id deve essere univoco
int checkRand(int num){

    int i;

    for (i=0;i<MAX_N_TAGS;i++){

        if (randId[i]==num){
            return 0;
        }

    }

    for (i=0;i<MAX_N_TAGS;i++){

        if (randId[i]==0){
            randId[i] = num;
            return 1;
        }

    }

    return -1;

}

void printArray(void){

    int i;

    printk("tagServiceArray: ");
    for (i=0;i<MAX_N_TAGS;i++){

        printk("%d\n",tagServiceArray[i]);

    }
}


int createId(void){

    int rand, res;
    generateRandomId:
        rand = 0;
        get_random_bytes(&rand, sizeof(int)-1); //excluding negative numbers 
        rand++;         //excluding 0
        rand%=256;      //upper bound is 256

        printk("rand = %d\n", rand);
            
        res = checkRand(rand);
        if (res == 0)
        {
            goto generateRandomId;
        } else if (res==-1)
        {
            printk("Error in checkRand\n");
            return -1;
        }
        else
        {
            return rand;
        }
        
    return rand;
}

int generateId(void){

    int id = createId();
    printk("randId[0]=%d\n",randId[0]);
    printk("randId[1]=%d\n",randId[1]);
    
    if (id == -1){
        printk("Errore dopo createId\n");
    }

    return id;

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
                return -1;  //non ho permesso perche utente è diverso 

            }
            if (tag->private==1){   //non posso fare open di tag privato
                printk("openTag: tag cannot be opened since it's private\n");
                return -1;
            }

            printk("openTag: returning tag id = %d\n",tag->ID);
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
    int id = generateId();
    newTag->ID = id;
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


/*
rifai check permessi, se livello è nel range ecc
*/
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId){
    
    tag_t* currTag; //get tag from ID

    if (level<0 || level>32){
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    currTag = getTagFromID(tagId);
    
    
    if (currTag->ID == -1){
        printk("errore. tag ha ID -1\n");
        return -1;
    }

    if (currTag->permission == 1 && currTag->creatorUserId != currentUserId){
        printk("errore. Utente %d non ha permesso di utilizzare questo tag\n", currentUserId);
        return -1;
    }


    printk("dentro deliverMsg: recuperato tag con ID %d a indirizzo %d\n",tagId,currTag);

    level_t** tagLevels= currTag->levels;
    printk("dentro deliverMsg: tagLevels= %d\n",tagLevels);
    printk("dentro deliverMsg: tagLevels[0]->num = %d\n", tagLevels[0]->num);
    printk("dentro deliverMsg: tagLevels[0]->msg = %s\n", tagLevels[0]->msg);

    tagLevels[level-1]->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);
    printk("dentro deliverMsg: tagLevels[level-1]->msg = %s\n", tagLevels[level-1]->msg);
    printk("dentro deliverMsg: msg= %s\n", msg);

    strcpy(tagLevels[level-1]->msg,msg);
    printk("dentro deliverMsg: tagLevels[level-1]->msg = %s\n",tagLevels[level-1]->msg);

    /*
    Ora devo creare lista thread in attesa e mandare un segnale
    che gli è arrivato msg dove funzione handler del segnale è la 
    receive (che sveglia i thread nella lista e leggono il msg)
    */

    //=====wait queue=====
    printk("dentro deliverMsg: tagLevels[level-1]->waitingThreads = %d\n",tagLevels[level-1]->waitingThreads);

    //qui bisogna svegliare i threads
    wake_up_interruptible(tagLevels[level-1]->waitingThreads);

    return 0;

}




void initRandIdArray(void){
    int i;

    for (i=0;i<MAX_N_TAGS;i++){
        randId[i] = -1;
    }

    return;
}

/*
se key sono uguali di 2 thread diversi (e key!=0) allora restituisco stesso tag
mentre se key=0 i tag devono essere diversi. e se faccio ipc private NON POSSO FARE OPEN!
quindi creo e basta il tag con key=0 e id generato randomicamente
*/

void serviceInitialization(void){
    printk("dentro serviceInitialization: randId[0]=%d\n",randId[0]);
    if (randId[0]!=0){
        printk("DOPO IF dentro serviceInitialization: randId[0]=%d\n",randId[0]);
        return;
    }
    else{
        initRandIdArray();
    }
    return;
}

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


