//
// Created by cecilia on 21/09/21.
//

#include <malloc.h>
#include "struct.h"

//qui faccio inizializzazione del sistema
//creo array per i tag , inizializzo livelli
//aggiungo tag all'array, lo rimuovo
//faccio istantiate del tag get

tag_t *tagServiceArray[MAX_N_TAGS];

/*
void initTagService(){

    tag_t *tagServiceArray[256];


}
 */
int getNumOfTags(){
    int len = sizeof tagServiceArray / sizeof tagServiceArray[0];
    printf("sizeof tagServiceArray = %d\n", sizeof tagServiceArray);
    printf("sizeof(tag_t) = %d\n", sizeof(tag_t));

    return len;

}
int addTag(int key, uid_t userId){

    tag_t* newTag;
    //newTag = kzalloc(sizeof(tag_t), GFP_KERNEL)




}

int main(){
    printf("hello world\n");
    printf("len array di tag= %d\n",getNumOfTags());

}

