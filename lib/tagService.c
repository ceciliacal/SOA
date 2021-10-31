#include "../include/tagService.h"
#include "../include/utils.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cecilia Calavaro <cecilia.calavaro@uniroma2.it>");
MODULE_DESCRIPTION("TAG_SERVICE");

tag_t *tagServiceArray[MAX_N_TAGS];
int global_nextId = 0;
int global_numTag = 0;

rwlock_t numTagLock;

rwlock_t tagLocks[MAX_N_TAGS];

tag_t** getTagServiceArray(void){
    return tagServiceArray;
}

rwlock_t* getTagLocks(void){
    return tagLocks;
}

void initTagLocks(void){
    int i;
    for (i=0;i<MAX_N_TAGS;i++){
        rwlock_init(&tagLocks[i]);
    }
}


void initLevels(level_t* levelsArray[N_LEVELS],spinlock_t levelLocks[N_LEVELS]){

    int i;

    for (i=0;i<N_LEVELS;i++){

        levelsArray[i]= (level_t*) kzalloc(sizeof(level_t),GFP_KERNEL);
        levelsArray[i]->numThreadsWq=0;
        levelsArray[i]->number=i+1;
        levelsArray[i]->wakeUpCondition=0;
        spin_lock_init(&levelLocks[i]);
          
        init_waitqueue_head(&(levelsArray[i]->waitingThreads));


    }

}


int openTag(int key, kuid_t currentUserId){

    int i;
    tag_t* tag;

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->key == key){
            
            tag = tagServiceArray[i];
            
            if (tag->permission == 1 && tag->creatorUserId.val!=currentUserId.val){
                printk(KERN_ERR"%s: openTag - permission denied\n",MODNAME);
                read_unlock(&tagLocks[i]);
                return -1;  //no permission because user is different 

            }
            if (tag->private==1){   //cannot open an IPC_PRIVATE tag 
                read_unlock(&tagLocks[i]);
                printk(KERN_ERR"%s: openTag - tag cannot be opened since it's private\n",MODNAME);
                return -1;
            }

            printk("%s: in openTag - returning tag id = %d\n",MODNAME,tag->ID);
            read_unlock(&tagLocks[i]);
            return tag->ID;

        }
        read_unlock(&tagLocks[i]);

    }

    return -1;   //not found

}


int checkAwakeAll(int tag, kuid_t currentUserId){

    int i;
    tag_t* currTag = NULL; //get tag from ID
    level_t* currLevel;

    for(i=0;i<MAX_N_TAGS;i++){
        
        read_lock(&tagLocks[i]);

        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tag){
            
            currTag = tagServiceArray[i];
            break;
            //printk("dentro getTagFromID: tagServiceArray[i] = %d\n", tagServiceArray[i]);
        }
        read_unlock(&tagLocks[i]);
    }

    if (currTag==NULL){
        printk(KERN_ERR"%s: in checkAwakeAll - tag with ID %d not found\n",MODNAME,tag);
        read_unlock(&tagLocks[i]);
        return -1;
    }

    if (checkPermission(currTag, currentUserId) == -1){
        printk(KERN_ERR"%s: in checkAwakeAll - permission denied\n",MODNAME);
        read_unlock(&tagLocks[i]);
        return -1;
    }

    //retrieve levels
    level_t** tagLevels= currTag->levels;
    
    //write lock on tag
    for (i=0;i<N_LEVELS;i++){

        if (tagLevels[i]!=NULL){

            spin_lock(&currTag->levelLocks[i]);

            level_t* currLevel = tagLevels[i];
            currLevel->wakeUpCondition=1;
            wake_up_all(&(currLevel->waitingThreads));


            spin_unlock(&currTag->levelLocks[i]);
        }
        
    }


    printk(KERN_INFO"%s: awake_all: threads in TAG=%d\n",MODNAME,currTag->numThreads);
    
    read_unlock(&tagLocks[i]);
    return 0;
}


void printArray(void){
    int i;
    printk("\n\n");
    for(i=0;i<MAX_N_TAGS;i++){
        
        if (tagServiceArray[i]!=NULL){
            printk("tagServiceArray[%d]=%d\n",i,tagServiceArray[i]->ID);
        }
    }
    printk("\n\n");
}

int waitForMessage(int tag,int level, char* buffer, size_t size, kuid_t uid){
    
    int resWaitEvent;
    int i;
    tag_t* currTag = NULL; //get tag from ID

    
    if (level<1 || level>32){
        printk(KERN_ERR"%s: in waitForMessage - level must be in range [1,32]\n",MODNAME);
        return -1;
    }

    int myLevel = level-1;

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tag){
            
            currTag = tagServiceArray[i];
            read_unlock(&tagLocks[i]);
            break;
        }

    }

    if (currTag==NULL){
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR"%s: in waitForMessage - tag with ID %d not found\n",MODNAME,tag);
        return -1;
    }

    if(checkPermission(currTag, uid)==-1){
        read_unlock(&tagLocks[i]);
        return -1;
    }

    printk(KERN_INFO"%s: in waitForMessage: got tag %d at %d\n",MODNAME,tag,currTag);

    level_t** tagLevels= currTag->levels;

    //ora metto thread in attesa di un msg 
    //+1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, +1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), +1);


    resWaitEvent = wait_event_interruptible(tagLevels[myLevel]->waitingThreads,tagLevels[myLevel]->wakeUpCondition==1);


    if (tagLevels[myLevel]->msg!=NULL&&tagLevels[myLevel]->wakeUpCondition==1){
        
        printk(KERN_INFO"%s: in waitForMessage - receiver thread woke up because of SEND\n",MODNAME);
        
    }
    else if (tagLevels[myLevel]->msg==NULL&&tagLevels[myLevel]->wakeUpCondition==1){

        printk(KERN_ERR"%s: in waitForMessage - receiver thread woke up because of AWAKE_ALL\n",MODNAME);

        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
        __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);
        return -1;

    }
    else {
    
        //-1 thread in sleep
        __sync_fetch_and_add(&currTag->numThreads, -1);
        __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);
        printk(KERN_ERR"%s: in waitForMessage - receiver thread woke up by a signal\n",MODNAME);

        return -1;

    }


    int resultCopy = __copy_to_user(buffer,tagLevels[myLevel]->msg, min(tagLevels[myLevel]->lastSize,size));
   
    //-1 thread in sleep
    __sync_fetch_and_add(&currTag->numThreads, -1);
    __sync_fetch_and_add(&(tagLevels[myLevel]->numThreadsWq), -1);

    //last receiver
    write_lock(&tagLocks[i]);
    if(tagLevels[myLevel]->numThreadsWq==0){

        spin_lock(&(currTag->levelLocks[myLevel]));
        
        //printk("\ndentro waitForMessage: NELL'IF - PRIMA NULL - msg=%s\n",tagLevels[myLevel]->msg);
        kfree(tagLevels[myLevel]->msg);
        tagLevels[myLevel]->msg=NULL;
        tagLevels[myLevel]->wakeUpCondition=0;

        //printk("\ndentro waitForMessage: NELL'IF - DOPO NULL - msg=%s\n",tagLevels[myLevel]->msg);

        spin_unlock(&(currTag->levelLocks[myLevel]));

    }


    write_unlock(&tagLocks[i]);
    printk(KERN_INFO"%s: end of waitForMessage: msg received from thread %d is: %s\n",MODNAME,uid.val, buffer);
    
    return 0;
    

}


int removeTag(int tag, kuid_t currentUserId){

    int i;
    
    if (global_numTag<1){
        printk(KERN_ERR"%s: removeTag ERROR: there are no tags to remove in the system.\n",MODNAME);
        return -1;
    }

    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID==tag){

            if (checkPermission(tagServiceArray[i], currentUserId) == -1){
                read_unlock(&tagLocks[i]);
                return -1;
            }         

            //check if there are any threads in wq
            if (tagServiceArray[i]->numThreads==0){

                read_unlock(&tagLocks[i]);

                write_lock(&tagLocks[i]);

                kfree(tagServiceArray[i]->levels);
                tagServiceArray[i]->levels = NULL;
                kfree(tagServiceArray[i]);
                tagServiceArray[i] = NULL;
                
                write_unlock(&tagLocks[i]);
                __sync_fetch_and_add(&global_numTag, -1);
                
                
                
                return 0;

            }
            
            else{

                read_unlock(&tagLocks[i]);
                printk(KERN_ERR"%s: removeTag ERROR: there are %d threads left waiting\n",MODNAME,tagServiceArray[i]->numThreads);
                return -1;
            }
            
        
        }
        read_unlock(&tagLocks[i]);

    }

    //no tag 
    printk(KERN_ERR"%s: removeTag ERROR: tag %d not found\n",MODNAME,tag);
    return -1;
}



int addTag(int key, kuid_t userId, int perm){

    int i;
    tag_t* newTag;

    if (global_numTag>255){
        printk(KERN_ERR"%s: addTag ERROR: tag service is full, impossible to add any further tag.\n",MODNAME);
        return -1;
    }

    if (key!=0){

        for(i=0;i<MAX_N_TAGS;i++){

            read_lock(&tagLocks[i]);
        
            if (tagServiceArray[i]!=NULL&&tagServiceArray[i]->key==key){
                printk(KERN_INFO"%s: addTag: there is already a tag with key %d.\n",MODNAME,key); 
                
                read_unlock(&tagLocks[i]);
                return -1;
            }
            read_unlock(&tagLocks[i]);
        }
    }

    
    //allocate new tag
    newTag = (tag_t*) kzalloc(sizeof(tag_t), GFP_KERNEL);
    //allocate levels for newTag
    level_t** levelsArray = (level_t**) kzalloc(sizeof(level_t*)*N_LEVELS,GFP_KERNEL);

    //set permission
    if (perm==1){
        newTag->permission = 1;
    } 
    else {
        newTag->permission = 0;
    }
    
    newTag->key = key;
    newTag->creatorUserId = userId;         //for permission
    newTag->numThreads = 0;                 //total number of threads in all 32 wait queues
    
    //level initialization
    initLevels(levelsArray, newTag->levelLocks);
    newTag->levels = levelsArray;

    //id initialization
    __sync_fetch_and_add(&global_nextId, +1);
    newTag->ID = global_nextId;

    //adding newTag to tagServiceArray
    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);

        if (tagServiceArray[i]==NULL){

            read_unlock(&tagLocks[i]);
            write_lock(&tagLocks[i]);
            tagServiceArray[i] = newTag;
            write_unlock(&tagLocks[i]);
            __sync_fetch_and_add(&global_numTag, +1);
            break;
        }

        read_unlock(&tagLocks[i]);

    }
    
    return newTag->ID;
    
}



int deliverMsg(int tagId, char* msg, int level, size_t size, kuid_t currentUserId){
    
    int i;
    tag_t* currTag = NULL; //get tag from ID


    for(i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);
        
        if (tagServiceArray[i]!=NULL && tagServiceArray[i]->ID == tagId){  
            currTag = tagServiceArray[i];
            
            break;
        }
        read_unlock(&tagLocks[i]);

    }
    

    if (currTag==NULL){
        printk(KERN_ERR"%s: deliverMessage ERROR: tag not found\n",MODNAME);
        read_unlock(&tagLocks[i]);
        return -1;
    }


    if (checkPermission(currTag, currentUserId) == -1){
        read_unlock(&tagLocks[i]);
        return -1;
    }
    
    printk(KERN_INFO"%s: deliverMsg: got tag with ID %d at %d\n",MODNAME,tagId,currTag);
        
    //retrieve specific level
    level_t** tagLevels= currTag->levels;
    level_t* currLevel = tagLevels[level-1];

    spin_lock(&currTag->levelLocks[level-1]);

    if(currLevel->msg != NULL){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR "%s: deliverMsg: level %d in tag %d is already in use. This SEND operation will be discarded.è\n",MODNAME,level,tagId);
        return -1;
    }

    currLevel->msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);

    currLevel->lastSize = size;
    
    //copy from user
    int copiati = __copy_from_user(currLevel->msg,msg,size);
    
    if (copiati!=0){
        spin_unlock(&currTag->levelLocks[level-1]);
        read_unlock(&tagLocks[i]);
        printk(KERN_ERR"%s: deliverMsg ERROR: copy_from_user went wrong!\n",MODNAME);
        return -1;
    }

    //=====wait queue=====
    //waking up threads waiting on this level
    currLevel->wakeUpCondition=1;
    wake_up_all(&(currLevel->waitingThreads));
    
    spin_unlock(&currTag->levelLocks[level-1]);
    read_unlock(&tagLocks[i]);
    
    return 0;

}

void cleanupTagService(void){

    int i,j;

    for (i=0;i<MAX_N_TAGS;i++){

        if (tagServiceArray[i]!=NULL){


            for (j=0;j<N_LEVELS;j++){

                if (tagServiceArray[i]->levels[j]->msg!=NULL){
                    kfree(tagServiceArray[i]->levels[j]->msg);
                }

                kfree(tagServiceArray[i]->levels[j]);
            }

            kfree(tagServiceArray[i]->levels);
            tagServiceArray[i]->levels = NULL;

            kfree(tagServiceArray[i]);
            tagServiceArray[i] = NULL;
                
        }

    }

    return;

} 