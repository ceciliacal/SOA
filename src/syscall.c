#include "initStruct.h"

int get_tag(int key, int command, int permission){

    int tagId;
    /*
    fai i check se tipo hai key=0 e open, ecc.
    */

    if (command==1){    //create

        //uid_t uid = getuid_call();
        uid_t uid = current_uid.val();
        pid_t pid = current->pid;
        tagId = addTag(key, uid, pid, permission);
        printk("CREATE: tagId = %d\n",tagId);

    }
    else{   //open

        



    }




}

/*
poi fare servizio bloccante x thread che provano ad accedere a stesso tag
in stesso momento!

*/