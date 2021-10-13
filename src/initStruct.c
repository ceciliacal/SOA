//
// Created by cecilia on 21/09/21.
//

#include "initStruct.h"

MODULE_LICENSE("GPL");

//qui faccio inizializzazione del sistema
//creo array per i tag , inizializzo livelli
//aggiungo tag all'array, lo rimuovo
//faccio istantiate del tag get

tag_t *tagServiceArray[MAX_N_TAGS];
int randId[MAX_N_TAGS];

void initLevels(level_t* levelsArray[N_LEVELS]){

    int i;

    for ( i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->num=i+1;
        printk("levelsArray[%d]= %d    num=%d\n",i,levelsArray[i],levelsArray[i]->num);

    }

    printk("levelsArray= %d\n",levelsArray);    //ind array
    printk("*levelsArray= %d\n",*levelsArray);  //primo elemen
    printk("&levelsArray= %d\n",&levelsArray);  //ind array
    printk("**levelsArray= %d\n",**levelsArray);  //ind array
    //return *levelsArray;
}

int checkRand(int num){

    int i;

    for (i=0;i<MAX_N_TAGS;i++){

        if (randId[i]==num){
            return 0;
        }

    }

    for (i=0;i<MAX_N_TAGS;i++){

        if (randId[i]==-1){
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
        get_random_bytes(&rand, sizeof(int)-1);
        rand%=256;

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

int openTag(int key, uid_t currentUserId, pid_t processId){

    int i;
    tag_t* tag;

    for(i=0;i<MAX_N_TAGS;i++){

        //do something. check della chiave scorrendo array di tag
        if (tagServiceArray[i]->key == key){

            tag = tagServiceArray[i];
            
            if (currentUserId==tag->creatorUserId && processId == tag->creatorUserId ){

                printk("dentro a opentag: tagServiceArray[0]->ID = %d\n",tag->ID);
    
                break;

            }

        }

    }

    return 0;
}

/*
pid non lo considero perché non posso richiamare tag_ctl 2 volte, quindi solo la
prima volta che creo il tag e quindi non ho la open. Il permission=1 solo se è CREATE
il command, perche nel testo dice che permission è usato per indicare per quale scopo
il tag è stato CREATO, quindi in open permission è sempre 0

*/
int openTag2(int key, uid_t currentUserId){

    int i;
    tag_t* tag;

    for(i=0;i<MAX_N_TAGS;i++){

        //do something. check della chiave scorrendo array di tag
        if (tagServiceArray[i]->key == key){

            tag = tagServiceArray[i];
            
            if (tag->permission == 1 && tag->creatorUserId!=currentUserId){
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
int addTag(int key, uid_t userId, pid_t creatorProcessId){

    tag_t* newTag;
    level_t* levels[N_LEVELS];
    level_t** levelsArray;

    //controllo che non esiste già una chiave uguale (se key!=0)
    /*
    -se qui sto facendo CREATE, e c'è gia chiave uguale ritorno -1 (user
    deve fare la OPEN con stessa chiave)
    -se key = 0, istanzio tag e procedo. 

    */
    //
    int i;
    for(i=0;i<MAX_N_TAGS;i++){
        printk("prima if: i=%d\n",i);
        if (tagServiceArray[i]==NULL){
            tagServiceArray[i] = newTag;
            printk("tagServiceArray[0]=%d\n",tagServiceArray[0]);
            break;
        }

    }
    
    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);

    newTag->key = key;
    //gestire il fatto che permission può essere = 1 o =0
    newTag->permission = 1; //todo: da cambiare!
    newTag->creatorUserId = userId;         //se permission=1
    newTag->creatorProc = creatorProcessId; 
    printk("dentro addTag: newTag->creatorUserId= %d\n",newTag->creatorUserId);
    printk("dentro addTag: newTag->creatorProc= %d\n",newTag->creatorProc);

    //inizializzazione livelli
    initLevels(levels);
    levelsArray = levels;
    
    printk("dentro addTag: levels= %d\n",levels);
    printk("dentro addTag:*levels= %d\n",*levels);

    newTag->levels = levelsArray;
    printk("dentro addTag: newTag->levels= %d\n",newTag->levels);
    printk("dentro addTag: newTag->levels[5].num= %d\n",newTag->levels[5]->num);

    //inizializzazione id
    int id = generateId();
    newTag->ID = id;
    printk("newTag->ID = %d\n", newTag->ID);


    if (key==0){

        newTag->private = 1;
    }
    else{

        newTag->private = 0;
    }

    printk("newTag-> private = %d\n", newTag->private);

    //aggiunto di newTag all'array
    printk("newTag=%d\n",newTag);
    
    for(i=0;i<MAX_N_TAGS;i++){
        printk("prima if: i=%d\n",i);
        if (tagServiceArray[i]==NULL){
            tagServiceArray[i] = newTag;
            printk("tagServiceArray[0]=%d\n",tagServiceArray[0]);
            break;
        }

    }
    
    return 1;
    
}


int addTag2(int key, uid_t userId, pid_t creatorProcessId, int perm){

    //controllo che non esiste già una chiave uguale (se key!=0)
    /*
    -se qui sto facendo CREATE, e c'è gia chiave uguale ritorno -1 (user
    deve fare la OPEN con stessa chiave)
    -se key = 0, istanzio tag e procedo. 

    */
    int i;

    if (key!=0){

        for(i=0;i<MAX_N_TAGS;i++){
        
            if (tagServiceArray[i]!=NULL&&tagServiceArray[i]->key==key){
                printk("addTag: there is already a tag with key %d\n. Try again with OPEN command.\n",tagServiceArray[i]->key); 
                return -1;
            }
        }
    }

    
    tag_t* newTag;
    level_t* levels[N_LEVELS];
    level_t** levelsArray;

    
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
    initLevels(levels);
    levelsArray = levels;
    
    printk("dentro addTag: levels= %d\n",levels);
    printk("dentro addTag:*levels= %d\n",*levels);

    newTag->levels = levelsArray;
    printk("dentro addTag: newTag->levels= %d\n",newTag->levels);
    printk("dentro addTag: newTag->levels[5].num= %d\n",newTag->levels[5]->num);

    //inizializzazione id
    int id = generateId();
    newTag->ID = id;
    printk("newTag->ID = %d\n", newTag->ID);


    //aggiunto di newTag all'array
    printk("newTag=%d\n",newTag);
    for(i=0;i<MAX_N_TAGS;i++){
        printk("prima if: i=%d\n",i);
        if (tagServiceArray[i]==NULL){
            tagServiceArray[i] = newTag;
            printk("tagServiceArray[0]=%d\n",tagServiceArray[0]);
            break;
        }

    }
    
    return newTag->ID;
    
}



void initRandIdArray(void){
    int i;

    for (i=0;i<MAX_N_TAGS;i++){
        randId[i] = -1;
        //printk("randId[%d] = %d\n", i, randId[i]);
    }
}

/*
ma la chiave quindi deve essere unica?? quella che mi arriva dallo user,
se ce n'è già una come faccio??

se key sono uguali di 2 thread diversi (e key!=0) allora restituisco stesso tag
mentre se key=0 i tag devono essere diversi. e se faccio ipc private NON POSSO FARE OPEN!
quindi creo e basta il tag con key=0 e id generato randomicamente


*/


int init_module(void){
    
    initRandIdArray();
    level_t* levels[N_LEVELS];
    
    addTag2(1,0,0,1);
    addTag2(2,0,0,0);
    int idtag= openTag2(1,0);
    int idtag1= openTag2(2,1);
    int idtag2= openTag2(1,1);
    printk("idtag=%d",idtag,"idtag1=%d",idtag1,"idtag2=%d\n",idtag2);
    
    return 0;
}



void cleanup_module(void){
    printk("CLEANUP!!\n");
    
}


