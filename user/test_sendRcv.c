#include "../include/const.h"
#include "user.h"
#include <string.h>

int numReceivers = 5;

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


// The function to be executed by all threads
void *createTag(void *vargp)
{
    int *myid = (int *)vargp;
    printf("createTag: thread %d\n", *myid);
    
    int id = syscall(get,0,CREATE,NO_PERMISSION);
  
    printf("Creato tag con ID %d dal thread %d\n",id,*myid);
    pthread_exit(NULL);
}




// The function to be executed by all threads
void *sendMsg(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sendMsg: thread %d\n", *myid);
    size_t size = 10;
    char* buffer = malloc(sizeof(char)*10);
    printf("sendMsg: buffer= %s\n", buffer);

    char* res;
    res=strncpy(buffer,"ciao",size);
    printf("sendMsg: res= %s\n", res);
    int r=syscall(send,1,1,buffer,size);
  
    if (r==0){
        printf("sendMsg: tag_send andata BENE!      thread %d ha inviato messaggio %s\n",*myid, buffer);
    }
    else{
        printf("tag_send result: %d. NON è andata bene!\n",r);
    }
    
    pthread_exit(NULL);
}

// The function to be executed by all threads
void *receiveMsg2(void *vargp)
{
    int *myid = (int *)vargp;
    //printf("receiveMsg: thread %d\n", *myid);
    size_t size = 10;
    char* buffer = malloc(sizeof(char)*size);



    printf("PRIMA tag_receive: thread %d , buffer= %s\n",*myid, buffer);
    int res = syscall(receive,1,1,buffer,size);
    printf("Syscall result: %d thread %d , buffer= %s\n",res, *myid, buffer);
    printf("DOPO tag_receive: thread %d , buffer= %s\n",*myid, buffer[0]);
    pthread_exit(NULL);
}

// The function to be executed by all threads
void *receiveMsg(rcv_args_t *info)
{
    //int *myid = (int *)vargp;
    //printf("receiveMsg: thread %d\n", *myid);
    //size_t size = 10;
    //char* buffer = malloc(sizeof(char)*size);

    printf("PRIMA tag_receive: thread %d , buffer= %s\n",info->tid, info->buffer);
    int res = syscall(receive,info->tag,info->level,info->buffer,info->size);
    printf("Syscall result: %d thread %d , buffer= %s\n",res, info->tid, info->buffer);
    //printf("DOPO tag_receive: thread %d , buffer= %s\n",*myid, buffer[0]);
    pthread_exit(NULL);
}

int main(int argc, char** argv){

    int i;
    pthread_t tid[numReceivers];
    size_t size = 10;
    rcv_args_t *info[numReceivers];

    

    /*
    qui bisognerebbe fare array di struct "info" 
    e poi passarle come parametro (..,..,..,info) nel for
    */

    //printf("\n---creazione threads CREATE:\n");
    int id = syscall(get,0,CREATE,NO_PERMISSION);
    

    //printf("\n---creazione threads RCV:\n");
    for (i=0; i<numReceivers; i++){
        //printf("---creazione threads -    tid[i]= %d\n",tid[i]);

        info[i] = malloc(sizeof(rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i;

        pthread_create(&tid[i], NULL, receiveMsg, info[i]);
        
    }

    //printf("\n---creazione threads SEND:\n");

    sleep(5);
    //printf("\n---FAI INTERRUPT!!!!!!:\n");

    char* msgToSent = "cacca";

    //char* buffer = malloc(sizeof(char)*size);
    //printf("sendMsg: msgToSent= %s\n", msgToSent);

    

    //res=strncpy(buffer,"cacca",size);
    int r=syscall(send,id,1,msgToSent,size);

    if (r==0){
        printf("sendMsg: tag_send andata BENE!  buffer= %s\n", msgToSent);
    }
    else{
        printf("tag_send result: %d. NON è andata bene!\n",r);
    }

    /*
    pthread_t tid1[2];
    for (i=0; i<2; i++){

        pthread_create(&tid1[i], NULL, sendMsg, (void *)&tid1[i]);
        
    }

    printf("\n---creazione threads RCV 2 VOLTA!:\n");
    pthread_t tid2[numReceivers];
    for (i=0; i<numReceivers; i++){

        pthread_create(&tid2[i], NULL, receiveMsg, (void *)&tid2[i]);
        
    }
    */





    for (i = 0; i < numReceivers; i++){
        pthread_join(tid[i],0);

    }

    /*
    for (i = 0; i < 2; i++){
        pthread_join(tid1[i],0);

    }
    for (i = 0; i < numReceivers; i++){
        pthread_join(tid2[i],0);

    }
    */


    return 0;

}


  

