#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct{
    
    pthread_t tid;

    int syscall;
    int tagId;
    int command;
    int permission;

} tag_get_args_t;




