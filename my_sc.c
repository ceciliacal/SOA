#include "./include/initStruct.h"
#include "./include/my_sc.h"

MODULE_LICENSE("GPL");

int get_tag(int key, int command, int permission){
	
    /*
    fai i check se tipo hai key=0 e open, ecc.
    fare check che numeri passati in command e permission
    siano effettivamente 0 o 1
    */

   printk("Dentro get_tag\n");

    
    int tagId;
    
    
    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

   
    serviceInitialization();
    if (command==1){    //create

        //uid_t uid = getuid_call();
        pid_t pid = current->pid;
        tagId = addTag(key, uid, pid, permission);
        printk("CREATE: tagId = %d\n",tagId);

    }
    else{   //open
        tagId = openTag(key, uid);

    }

    return tagId;
    
   
}

/*
poi fare servizio bloccante x thread che provano ad accedere a stesso tag
in stesso momento!


Ma se io voglio testare modulo user mode prima per forza devo montare
la syscall nella syscall table?????????????
*/

int init_module(void){
    
    printk("dentro my_sc: in init_module\n");
    int id = get_tag(1,1,0);
    

    return 0;  
    
}



void cleanup_module(void){
    printk("CLEANUP!\n");
    
}
