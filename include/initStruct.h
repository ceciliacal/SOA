#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/wait.h>
#include "linux/string.h"
#include "const.h"


typedef struct{
    
    char *msg;
    int num;
    wait_queue_head_t* waitingThreads;

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
void serviceInitialization(void);
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId);
void addElemToLevel(void);
tag_t* getTagFromID(int id);
int removeTag(int tag);
void printArray(void);
int waitForMessage(int tag,int myLevel, char* buffer, size_t size,kuid_t uid);


