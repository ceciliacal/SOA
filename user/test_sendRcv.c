#include "../include/const.h"
#include "user.h"
#include <string.h>

int numReceivers = 5;
size_t size;


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
void *receiveMsg(void *vargp)
{
    int *myid = (int *)vargp;
    printf("receiveMsg: thread %d\n", *myid);
    //size_t size = 10;
    char* buffer = malloc(sizeof(char)*size);

    syscall(receive,1,1,buffer,size);
  
    printf("FINE receiveMsg: thread %d , buffer= %s\n",*myid, buffer);
    pthread_exit(NULL);
}


// The function to be executed by all threads
void *sendMsg(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sendMsg: thread %d\n", *myid);
    size_t size = 10;
    char* buffer = malloc(sizeof(char)*20);
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



int main(int argc, char** argv){

    int i;
    pthread_t tid[numReceivers];
    

    struct tag_get_args_t *info  = malloc(sizeof(tag_get_args_t));
    /*
    qui bisognerebbe fare array di struct "info" 
    e poi passarle come parametro (..,..,..,info) nel for
    */

    printf("\n---creazione threads CREATE:\n");
    int id = syscall(get,0,CREATE,NO_PERMISSION);
    /*
    for (i=0; i<numReceivers; i++){

        pthread_create(&tid[i], NULL, createTag, (void *)&tid[i]);
        
    }
    */

    //printf("FINE FOR!!!!!!!!\n");
    printf("\n\n inizio sleep...\n");
    sleep(3);
    
    printf("\n\n fine sleep...\n generazione dei thread per receive!\n");

    printf("\n---creazione threads RCV:\n");
    for (i=0; i<numReceivers; i++){

        pthread_create(&tid[i], NULL, receiveMsg, (void *)&tid[i]);
        
    }

    sleep(3);

    printf("\n\n fine sleep...\n generazione dei thread per send!\n");

    printf("\n---creazione threads SEND:\n");

    //size_t size = 10;
    

    char* res;
    char* msgToSent = "cacca";
    size = sizeof(msgToSent);
    printf("size=%d\n", size);

    char* buffer = malloc(sizeof(char)*size);
    printf("sendMsg: buffer= %s\n", buffer);


    res=strncpy(buffer,"cacca",size);
    int r=syscall(send,1,1,buffer,size);

    if (r==0){
        printf("sendMsg: tag_send andata BENE!  buffer= %s\n", buffer);
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


  

