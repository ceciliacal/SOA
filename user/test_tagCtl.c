#include "user.h"
#include <string.h>

int numReceivers = 30;
int numSenders = 2;

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

    if ((info->buffer)[0]=='\0'){
        printf("thread %d - Syscall tag_receive result: %d   -  SUCCESS, buffer is empty\n",info->tid,res);
    }

    pthread_exit(NULL);
}


int main(int argc, char** argv){

    int i;
    pthread_t tidR[numReceivers];
    pthread_t tidS[numSenders];
    size_t size = 10;
    send_rcv_args_t *info[numReceivers];

    //create tag
    int id = syscall(get,0,CREATE,NO_PERMISSION);

    //spawn receiver threads
    for (i=0; i<numReceivers; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));
        char* buffer = malloc(sizeof(char)*10);
        
        info[i]->buffer=buffer;
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=i+1;
        info[i]->tid=i;

        pthread_create(&tidR[i], NULL, receiveMsg, info[i]);
        
    }

    sleep(5);

    
    //spawn sender threads
    for (i=0; i<2; i++){

        info[i] = malloc(sizeof(send_rcv_args_t));

        //same msg per iteration
        info[i]->buffer= "test msg";    
        info[i]->size=size;
        info[i]->tag=id;
        info[i]->level=i+1; //same tag but every thread has a different level
        info[i]->tid=i;

        pthread_create(&tidS[i], NULL, sendMsg, info[i]);        
    }

    //We spawned 30 receivers and 2 sender. 
    //sender threads sent their message respectively on level 1 and 2 so two receiver threads (those at level 1 and 2) 
    //consumed their message while the remaining 28 threads are still waiting on their level.
    
    //Now we'll call syscall tag_get using commands REMOVE (to make sure tag won't be deleted) 
    //and  AWAKE_ALL (to make sure every thread wakes up, regardless its level)

    sleep(5);
    int resRemove = syscall(ctl,id,REMOVE);
    
    if (resRemove==0){
        printf("Syscall tag_ctl (REMOVE) result: %d  -  ERROR: tag %d was successfully removed\n", resRemove, id);
    }else if (resRemove==-1){
        printf("Syscall tag_ctl (REMOVE) result: %d  -  SUCCESS: tag cannot be removed\n", resRemove, id);
    }

    sleep(3);

    int resAwake = syscall(ctl,id,AWAKE_ALL);
    if (resAwake==0){
        printf("Syscall tag_ctl (AWAKE_ALL) result: %d  -  SUCCESS: threads in tag %d were successfully awaken\n", resAwake, id);
    }else if (resAwake==-1){
        printf("Syscall tag_ctl (AWAKE_ALL) result: %d  -  ERROR\n", resAwake, id);
    }


    for (i=0; i<numReceivers; i++){
        pthread_join(tidR[i],0);
    }

    for (i=0; i<numSenders; i++){
        pthread_join(tidS[i],0);
    }

    return 0;

}


  

