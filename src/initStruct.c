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

    printk("randId: ");
    for (i=0;i<MAX_N_TAGS;i++){

        printk("%d\n",randId[i]);

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


//int addTag(int key, uid_t userId){
int addTag(int key){

    tag_t* newTag;
    level_t* levels[N_LEVELS];
    level_t** levelsArray;
    
    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);
    
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
    
    return 1;
    
}


void initRandIdArray(void){
    int i;

    for (i=0;i<MAX_N_TAGS;i++){
        randId[i] = -1;
        //printk("randId[%d] = %d\n", i, randId[i]);
    }
}




int init_module(void){
    
    initRandIdArray();
    level_t* levels[N_LEVELS];
    addTag(1);
    
    return 0;
}



void cleanup_module(void){
    printk("CLEANUP!!\n");
    
}


