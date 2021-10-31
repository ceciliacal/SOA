#include "user.h"

int numReceivers = 5;


void *sendMsg(send_rcv_args_t *info)
{
    printf("Before syscall tag_send: thread %d sending msg = %s %s\n",info->tid, info->buffer);
    int r=syscall(send,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_send result: %d thread %d\n",r, info->tid);
    
    
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
    pthread_t tid[numReceivers];
    size_t size = 10;
    send_rcv_args_t *info[numReceivers];

    char* msgToSent = "cacca";
    
    int id = syscall(get,0,CREATE,NO_PERMISSION);

    //test1: 5 thread receivers concorrenti leggono messaggio mandato da un sender su stesso tag e stesso livello
    
    for (i=0; i<numReceivers; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=1;
        info[i]->tid=i;

        pthread_create(&tid[i], NULL, receiveMsg, info[i]);
        
    }

    sleep(5);


    int r=syscall(send,id,1,msgToSent,size);

    if (r==0){
        printf("sendMsg: tag_send r andata BENE!  buffer= %s\n", msgToSent);
    }
    else{
        printf("tag_send result: r %d. NON è andata bene!\n",r);
    }
    

    
    for (i = 0; i < numReceivers; i++){
        pthread_join(tid[i],0);

    }
    

    
    return 0;

}


  

