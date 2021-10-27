#define EXPORT_SYMTAB
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cred.h>
//#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/rwlock.h>
#include <linux/rwlock_api_smp.h>
#include "const.h"

typedef struct{
    
    char *msg;
    int numThreadsWq;
    wait_queue_head_t waitingThreads; 
    ssize_t lastSize;  //size of last sent message to this level

} level_t;

typedef struct {
    int key;
    int permission;
    kuid_t creatorUserId;   //serve per permission
    int ID;
    int private;
    pid_t creatorProc;  //serve per IPC (tag privato tra i thread di stesso processo)

    //pointer a array per livelli
    level_t **levels;
    int numThreads;         //num of threads in every level's waitqueue
    spinlock_t levelLocks[N_LEVELS];

} tag_t;

int addTag(int key, kuid_t userId, pid_t creatorProcessId, int perm);
int openTag(int key, kuid_t currentUserId);
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId);
int removeTag(int tag, kuid_t currentUserId);
void printArray(void);
int waitForMessage(int tag,int myLevel, char* buffer, size_t size,kuid_t uid);
void initTagLocks(void);
int checkAwakeAll(int tag, kuid_t currentUserId);

