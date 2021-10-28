#include "../include/const.h"
#include "user.h"
#include <string.h>

int numReceivers = 200;
int numSenders = 20;

/*
TODO: testare
    -rcv su due livelli diversi di stesso tag
    - permessi?!
    -3 msg diversi di fila
    - for con le send
    -size?!
    -testare awake all dopo sleep (1 rcv, sleep, awake all)
    -casistiche tag get con permessi, open ecc
    -rmv quando ci sono thread nella wq
*/


void *sendMsg(rcv_args_t *info)
{
    printf("Before syscall tag_send: thread %d sending msg = %s %s\n",info->tid, info->buffer);
    int res=syscall(send,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_send result: %d thread %d\n",res, info->tid);
    
    
    pthread_exit(NULL);
}

void *receiveMsg(rcv_args_t *info){
    
    printf("Before syscall tag_receive: thread %d , buffer= %s\n",info->tid, info->buffer);
    int res = syscall(receive,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_receive result: %d thread %d , buffer= %s\n",res, info->tid, info->buffer);

    pthread_exit(NULL);
}

int main(int argc, char** argv){

    int i;
    pthread_t tidR[numReceivers];
    pthread_t tidS[numSenders];
    size_t size = 10;
    rcv_args_t *info[numReceivers];

    //create tag
    int id = syscall(get,0,CREATE,NO_PERMISSION);

    //spawn receiver threads
    for (i=0; i<numReceivers; i++){

        info[i] = malloc(sizeof(rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i;

        pthread_create(&tidR[i], NULL, receiveMsg, info[i]);
        
    }

    sleep(5);

    
    //spawn sender threads
    for (i=0; i<2; i++){

        info[i] = malloc(sizeof(rcv_args_t));

        //different msg per iteration: "msg+i" (i=0,1,...)
        char* myMsg = malloc(sizeof(char)*size);
        char intToStr[12];
        
        strcpy(myMsg,"msg");       
        sprintf(intToStr,"%d",i);
        strcat(myMsg,intToStr);
        
        info[i]->buffer= myMsg;    
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i;

        pthread_create(&tidS[i], NULL, sendMsg, info[i]);        
    }

   
    for (i=0; i<numReceivers; i++){
        pthread_join(tidR[i],0);
    }

    for (i=0; i<numSenders; i++){
        pthread_join(tidS[i],0);
    }

    return 0;

}


  

