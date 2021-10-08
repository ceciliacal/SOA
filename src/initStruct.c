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


//int addTag(int key, uid_t userId){
int addTag(int i){

    tag_t* newTag;
    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);

    level_t* levels[N_LEVELS];
    
    initLevels(levels);
    printk("dentro addTag (dopo initLevels): levels= %d\n",levels);
    //printk("dentro addTag: newTag.levels= %d\n",newTag->levels);
    
    return 1;
    
}






int init_module(void){
    
    level_t* levels[N_LEVELS];
    addTag(1);
    
    return 0;
}



void cleanup_module(void){
    printk("CLEANUP!!\n");
    
}


