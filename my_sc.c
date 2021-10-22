#include "./include/initStruct.h"
#include "./include/const.h"
#include "linux/string.h"

MODULE_LICENSE("GPL");

int tag_get(int key, int command, int permission){
	
    /*
    fai i check se tipo hai key=0 e open, ecc.
    fare check che numeri passati in command e permission
    siano effettivamente 0 o 1
    */

    printk("Dentro get_tag\n Inizializzazione del servizio...\n");
    
    int tagId;
    
    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (command=CREATE){    //create

        pid_t pid = current->pid;
        tagId = addTag(key, uid, pid, permission);
        printk("in my_sc: CREATE -> tagId = %d\n",tagId);

    }
    else{   //open
        tagId = openTag(key, uid);

    }
    printk("====== end tag_get with %d ======\n",tagId);

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

int tag_receive(int tag, int level, char* buffer, size_t size), 
this service allows a thread to call the blocking receive operation of the message
to be taken from the corresponding tag descriptor at a given level. 
The operation can fail also because of the delivery of a Posix signal
 to the thread while the thread is waiting for the message.
*/


int tag_send(int tag, int level, char* buffer, size_t size){

    char* msg;

    if (size > MAX_MSG_SIZE){
        printk("ERROR: msg size exceeded maximum lenght");
        return -1;
    }

    if (size == 0){
        size = 1;
    }

    printk("buffer. buffer =%s\n",buffer);
    msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);
    //copy_from_user(msg, buffer, size);
    strcpy(msg, buffer);
    printk("msg allocato. msg =%s\n",msg);

    deliverMsg(tag, msg, level, size);
    
    return 0;

}




int init_module(void){
    
    printk("dentro my_sc: in init_module\n");
    int id = tag_get(1,CREATE,NO_PERMISSION);
    int id1 = tag_get(1,CREATE,PERMISSION);
    int id2 = tag_get(0,CREATE,PERMISSION);
    int id3 = tag_get(0,CREATE,PERMISSION);
    printk("dentro my_sc: in init_module -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);

    int retSend = tag_send(id, 1, "ciao", 5);
    printk("dentro my_sc: retSend= %d\n", retSend);
    return 0;  

    
    
}



void cleanup_module(void){
    printk("CLEANUP!\n");
    
}
