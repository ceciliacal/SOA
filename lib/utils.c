#include "../include/initStruct.h"

MODULE_LICENSE("GPL");

int checkCorrectCondition(tag_t* tag, kuid_t currentUserId){

    if (tag->ID == -1){
        printk("errore. tag ha ID -1\n");
        return -1;
    }

    if (tag->permission == 1 && tag->creatorUserId.val != currentUserId.val){
        printk("errore. Utente %d non ha permesso di utilizzare questo tag\n", currentUserId);
        return -1;
    }

    return 0;

}

int checkBufferSize(size_t size){

    if (size > MAX_MSG_SIZE){
        printk("ERROR: msg size exceeded maximum lenght");
        return -1;
    }

    if (size == 0){
        size = 1;
    }

    return 0;

}
