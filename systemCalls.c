#include "./include/tagService.h"
#include "./include/const.h"
#include "./include/utils.h"


MODULE_LICENSE("GPL");

int tag_get(int key, int command, int permission){

    int tagId;

    if (command!=CREATE && command!=OPEN||permission!=PERMISSION && permission!=NO_PERMISSION){
        printk(KERN_ERR"%s: tag_get: both COMMAND and PERMISSION value must be either 0 or 1.\n",MODNAME);
        return -1;
    }

    if (key==0 && command==OPEN){
        printk(KERN_ERR"%s: tag_get: cannot open a tag with IPC_PRIVATE key.\n",MODNAME);
        return -1;
    }

    printk(KERN_INFO"%s: tag_get: Creating new tag...\n",MODNAME);
    
    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (command==CREATE){    //create

        tagId = addTag(key, uid, permission);
        printk(KERN_INFO"%s: tag_get CREATE -> tagId = %d\n",MODNAME,tagId);

    }

    else{                   //open
        tagId = openTag(key, uid);

    }
    printk(KERN_INFO"%s:====== end tag_get with %d ======\n",MODNAME,tagId);


    return tagId;
    
}


/*int tag_send(int tag, int level, char* buffer, size_t size), 
this service delivers to the TAG service with tag as the descriptor the message 
currently located in the buffer at address and made of size bytes.
 All the threads that are currently waiting for such a message on the 
 corresponding value of level should be resumed for execution and should 
 receive the message (zero lenght messages are anyhow allowed).
  The service does not keep the log of messages that have been sent, 
  hence if no receiver is waiting for the message this is simply discarded.
*/


int tag_send(int tag, int level, char* buffer, size_t size){

    char* msg;
    int res;


    if (level<1 || level>32){
        printk(KERN_ERR"%s: tag_send ERROR: level inserted is wrong\n",MODNAME);
        return -1;
    }

    int checkSize = checkBufferSize(size);

    if (checkSize==1){
        size = 1;
    }
    else if(checkSize==-1){
        return -1;
    }

    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    res = deliverMsg(tag, buffer, level, size, uid);

    if (res==-1){
        
        printk(KERN_ERR"%s: tag_send ERROR: deliverMsg went wrong\n",MODNAME);
        return -1;
    }
    
    return 0;

}

/*
int tag_receive(int tag, int level, char* buffer, size_t size), 
this service allows a thread to call the blocking receive operation of the message
to be taken from the corresponding tag descriptor at a given level. 
The operation can fail also because of the delivery of a Posix signal
to the thread while the thread is waiting for the message.
*/
int tag_receive(int tag, int level, char* buffer, size_t size){

    int res;

    if (level<1 || level>32){
        printk(KERN_ERR"%s: tag_receive ERROR: level inserted is wrong\n",MODNAME);
        return -1;
    }

    int checkSize = checkBufferSize(size);
    if (checkSize==-1){
        return -1;
    }else if(checkSize==1){
        size = 1;

    }


    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (waitForMessage(tag,level,buffer,size,uid)==-1){
        printk(KERN_ERR"%s: tag_receive ERROR: waitForMessage went wrong\n",MODNAME);
        return -1;
    }
    else{
        printk(KERN_INFO"%s: tag_receive: end of function. buffer = %s\n",MODNAME, buffer);
    }

    return 0;
}

/*
int tag_ctl(int tag, int command),
this system call allows the caller to control the TAG service with tag
as descriptor according to command that can be either AWAKE_ALL 
(for awaking all the threads waiting for messages, independently of the level),
or REMOVE(for removing the TAG service from the system). 
A TAG service cannot be removed if there are threads waiting for messages on it.
By default, at least 256 TAG services should be allowed to be handled by software. 
Also, the maximum size of the handled message should be of at least 4 KB.
*/

int tag_ctl(int tag, int command){

    int res;

    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (command!=REMOVE && command!=AWAKE_ALL){
        printk(KERN_ERR"%s: tag_ctl ERROR: command must be either REMOVE (1) or AWAKE_ALL(0)\n",MODNAME);
            return -1;
    }
    if (command==REMOVE){

        res = removeTag(tag, uid);
        if (res==-1){
            printk(KERN_ERR"%s: tag_ctl ERROR: removeTag went wrong\n",MODNAME);

            return -1;
        }

        printk(KERN_INFO"%s: tag_ctl: end of function. Successfully removed tag %d\n",MODNAME,tag);
    } else if (command==AWAKE_ALL) {

        res = checkAwakeAll(tag,uid);
        if (res==-1){
            printk(KERN_ERR"%s: tag_ctl ERROR: checkAwakeAll went wrong\n",MODNAME);
            return -1;
        }
        printk(KERN_INFO"%s: tag_ctl: end of function. Successfully performed awake_all\n",MODNAME);

    }

    return 0;
}



