#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../include/const.h"

typedef struct{
    
    pthread_t tid;

    int tag;
    int level;
    char* buffer;
    size_t size;
    

} send_rcv_args_t;




