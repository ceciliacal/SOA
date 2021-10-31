#define EXPORT_SYMTAB
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/string.h>
#include <linux/rwlock.h>
#include <linux/rwlock_api_smp.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>

#include "const.h"

typedef struct{
    
    char *msg;
    int numThreadsWq;                   //number of threads in wait queue
    wait_queue_head_t waitingThreads;   //wait queue
    ssize_t lastSize;                   //size of last sent message to this level
    int wakeUpCondition;                //condition to set before AWAKE_ALL (check in tag_receive)
    int number;                         //level number (1...32)

} level_t;

typedef struct {
    int key;
    int permission;
    kuid_t creatorUserId;               //serve per permission
    int ID;
    int private;

    level_t **levels;                   //pointer a array per livelli
    int numThreads;                     //num of threads in every level's waitqueue
    spinlock_t levelLocks[N_LEVELS];

} tag_t;

int addTag(int key, kuid_t userId, int perm);
int openTag(int key, kuid_t currentUserId);
int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId);
int removeTag(int tag, kuid_t currentUserId);
void printArray(void);
int waitForMessage(int tag,int myLevel, char* buffer, size_t size,kuid_t uid);
void initTagLocks(void);
int checkAwakeAll(int tag, kuid_t currentUserId);
tag_t** getTagServiceArray(void);
rwlock_t* getTagLocks(void);

