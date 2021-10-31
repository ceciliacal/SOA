#include "../include/tagService.h"

MODULE_LICENSE("GPL");

int checkCorrectCondition(tag_t* tag, kuid_t currentUserId){

    if (tag->permission == 1 && tag->creatorUserId.val != currentUserId.val){
        printk("errore. Utente %d non ha permesso di utilizzare questo tag\n", currentUserId);
        return -1;
    }

    return 0;

}

int checkBufferSize(size_t size){

    if (size<0){
        printk("ERROR: insert msg size between 0 and %d\n",MAX_MSG_SIZE);
        return -1;
    }

    if (size>MAX_MSG_SIZE){
        printk("ERROR: msg size exceeded maximum lenght\n");
        return -1;
    }

    if (size == 0){
        printk("tag_receive, sto in checkBufferSize: size=0\n");
        return 1;
    }

    return 0;

}
