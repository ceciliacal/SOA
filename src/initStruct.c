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

/*
void initTagService(){

    tag_t *tagServiceArray[256];


}
 
int getNumOfTags(){
    int len = sizeof tagServiceArray / sizeof tagServiceArray[0];
    //printf("sizeof tagServiceArray = %d\n", sizeof tagServiceArray);
    //printf("sizeof(tag_t) = %d\n", sizeof(tag_t));

    return len;

}
*/

int addTag(int key, uid_t userId){

    tag_t* newTag;
    

    //prima devo inizializzare livelli
    void *buf;

    printf("sizeof(tag_t) = %d\n", sizeof(tag_t));
    newTag = kzalloc(sizeof(tag_t), GFP_KERNEL);

    return 0;




}

void initLevels(int num){

    level_t* levelsArray[N_LEVELS];
    int i;

    for ( i=0;i<N_LEVELS;i++){

        //level_t* newLev;
        //newLev = kzalloc(sizeof(level_t),GFP_KERNEL);

        levelsArray[i]= kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->num=i+1;
        printk("levelsArray[%d]= %d    num=%d\n",i,levelsArray[i],levelsArray[i]->num);

    }

}

/*
Il check se chiave è ipc private, collegamento degli ID/key ecc va implementato 
a livello kernel. 

e pure per scrivere msg da user a kernel mesa che serve char device driver


*/

int init_module(){
    
    initLevels(1);

    return 0;
}

