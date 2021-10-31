#include "user.h"
#include <string.h>


// The function to be executed by all threads
void *receiveMsg(send_rcv_args_t *info){

    printf("PRIMA tag_receive: thread %d , buffer= %s\n",info->tid, info->buffer);
    int res = syscall(receive,info->tag,info->level,info->buffer,info->size);
    printf("Syscall tag_receive result: %d thread %d , buffer= %s\n",res, info->tid, info->buffer);
    pthread_exit(NULL);

}

int main(int argc, char** argv){

    int i;
    size_t size = 1000;

    //char* msgToSent = "cacca";
    char* buffer = malloc(sizeof(char)*size);
    
    printf("Creating tag...\n");
    int id = syscall(get,0,CREATE,NO_PERMISSION);
    
    if (id<1){
        printf("Error during creation of tag!\n");
        return -1;
    }
    
    printf("Tag created with id=%d\n",id);


    sleep(2);
    int res_rcv = syscall(receive,id,1,buffer,size);

    printf("tag_receive result=%d   buffer=%s\n",res_rcv,buffer);

    return 0;

}


  

