#include "../include/tagService.h"

MODULE_LICENSE("GPL");

int checkPermission(tag_t* tag, kuid_t currentUserId){

    if (tag->permission == 1 && tag->creatorUserId.val != currentUserId.val){
        printk(KERN_ERR"%s: checkPermission ERROR: permission denied to user %d\n",MODNAME,currentUserId.val);
        return -1;
    }

    return 0;

}

int checkBufferSize(size_t size){



    if (size<0){
        printk(KERN_ERR"%s: checkBufferSize ERROR: insert msg size between 0 and %d\n",MODNAME,MAX_MSG_SIZE);
        return -1;
    }

    if (4096<size){
        printk(KERN_ERR"%s: checkBufferSize ERROR: msg size exceeded maximum lenght\n",MODNAME);
        return -1;
    }

    if (size == 0){
        return 1;
    }

    return 0;

}
