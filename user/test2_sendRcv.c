#include "user.h"

int numReceivers = 15;
int numSenders = 5;


void *sendMsg(send_rcv_args_t *info)
{
    printf("Before syscall tag_send: thread %d sending msg = %s %s\n",info->tid, info->buffer);
    int res=syscall(send,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_send result: %d thread %d\n",res, info->tid);
    
    
    pthread_exit(NULL);
}

void *receiveMsg(send_rcv_args_t *info){
    
    printf("Before syscall tag_receive: thread %d , buffer= %s\n",info->tid, info->buffer);
    int res = syscall(receive,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_receive result: %d thread %d , buffer= %s\n",res, info->tid, info->buffer);

    pthread_exit(NULL);
}

int main(int argc, char** argv){

    int i;
    pthread_t tidR1[numReceivers];
    pthread_t tidS1[numSenders];
    pthread_t tidR2[numReceivers];
    pthread_t tidS2[numSenders];
    size_t size = 10;
    send_rcv_args_t *info[numReceivers];

    //create tag
    int id = syscall(get,0,CREATE,NO_PERMISSION);

    //===========FIRST ROUND===========

    //spawn receiver threads
    for (i=0; i<numReceivers; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i;

        pthread_create(&tidR1[i], NULL, receiveMsg, info[i]);
        
    }

    sleep(3);

    printf("\n\n");
  
    //spawn sender threads
    for (i=0; i<numSenders; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));

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

        pthread_create(&tidS1[i], NULL, sendMsg, info[i]);        
    }

   //===========SECOND ROUND=========== 

    printf("\n\n");
    sleep(6);   
    //spawn receiver threads
    for (i=0; i<numReceivers; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=numReceivers+i;

        pthread_create(&tidR2[i], NULL, receiveMsg, info[i]);
        
    }

    sleep(3);
    printf("\n\n");
  
    //spawn sender threads
    for (i=0; i<numSenders; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));

        //different msg per iteration: "msg+i" (i=0,1,...)
        char* myMsg = malloc(sizeof(char)*size);
        char intToStr[12];
        
        strcpy(myMsg,"secondMsg");       
        sprintf(intToStr,"%d",i);
        strcat(myMsg,intToStr);
        
        info[i]->buffer= myMsg;    
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i+numSenders;

        pthread_create(&tidS2[i], NULL, sendMsg, info[i]);        
    }


   
    for (i=0; i<numReceivers; i++){
        pthread_join(tidR1[i],0);
    }

    for (i=0; i<numSenders; i++){
        pthread_join(tidS1[i],0);
    }

    for (i=0; i<numReceivers; i++){
        pthread_join(tidR2[i],0);
    }

    for (i=0; i<numSenders; i++){
        pthread_join(tidS2[i],0);
    }

    return 0;

}


  

