//
// Created by cecilia on 21/09/21.
//

//#include <unistd.h>
//#include <stdio.h>
//#include <pthread.h>
//#include <stdlib.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/slab.h>


#define MAX_N_TAGS 256
#define N_LEVELS 32
#define MAX_MSG_SIZE 4096

typedef struct{
    //array msg
    //pointer a array tid in attesa
    char *msg;
    int num;
    //pthread_t *waitingThreads;

} level_t;

typedef struct {
    int key;
    //int permission;
    uid_t permission;
    int ID;
    pid_t creatorProc;

    //pointer a array per livelli
    level_t *levels[32];
} tag_t;


