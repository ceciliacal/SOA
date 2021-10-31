#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct{
    
    pthread_t tid;

    int tag;
    int level;
    char* buffer;
    size_t size;
    

} rcv_args_t;




