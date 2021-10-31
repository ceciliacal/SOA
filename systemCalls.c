#include "./include/tagService.h"
#include "./include/const.h"
#include "./include/utils.h"

//TODO: free struct nella cleanup del modulo

MODULE_LICENSE("GPL");

int tag_get(int key, int command, int permission){
	
    /*
    fai i check se tipo hai key=0 e open, ecc.
    fare check che numeri passati in command e permission
    siano effettivamente 0 o 1
    */

    if (command!=CREATE && command!=OPEN||permission!=PERMISSION && permission!=NO_PERMISSION){
        printk("in tag_get: both COMMAND and PERMISSION value must be either 0 or 1.\n");
        return -1;
    }

    if (key==0 && command==OPEN){
        printk("in tag_get: cannot open a tag with IPC_PRIVATE key.\n");
        return -1;
    }

    printk("Dentro get_tag\n Inizializzazione del servizio...\n");
    
    int tagId;
    
    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (command==CREATE){    //create

        pid_t pid = current->pid;
        tagId = addTag(key, uid, permission);
        printk("in tag_get: CREATE -> tagId = %d\n",tagId);

    }
    else{   //open
        tagId = openTag(key, uid);

    }
    printk("====== end tag_get with %d ======\n",tagId);

    if (tagId==104){
        printArray();
    }
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
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    int checkSize = checkBufferSize(size);

    if (checkSize==1){
        size = 1;
    }
    else{
        printk("ERROR: msg size exceeded maximum lenght");
        return -1;
    }


    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    res = deliverMsg(tag, buffer, level, size, uid);

    if (res==-1){
        
        printk("ERROR: error during deliver msg");
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

    //questo lo lascio qua cosi se livello sbagliato manco cerco tag
    if (level<1 || level>32){
        printk("errore. Livello inserito è errato\n");
        return -1;
    }

    int checkSize = checkBufferSize(size);
    if (checkSize==-1){
        printk("errore. Dimensione messaggio è troppo grande\n");
        return -1;
    }else if(checkSize==1){
        size = 1;

    }


    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (waitForMessage(tag,level,buffer,size,uid)==-1){
        printk("waitForMessage: errore, return =-1\n");
        return -1;
    }
    else{
        printk("waitForMessage: fine funzione. buffer = %s\n",buffer);
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
        printk("ERROR in tag_ctl: command must be either REMOVE (1) or AWAKE_ALL(0)\n");
            return -1;
    }
    if (command==REMOVE){

        res = removeTag(tag, uid);
        if (res==-1){
            printk("ERROR in tag_ctl: remove è andata MALE!\n");
            return -1;
        }

        printk("FINE in tag_ctl: rimosso tag con ID %d\n",tag);
    } else if (command==AWAKE_ALL) {

        res = checkAwakeAll(tag,uid);
        if (res==-1){
            printk("ERROR in tag_ctl: AWAKE_ALL è andata MALE!\n");
            return -1;
        }


    }

    return 0;
}




/*
int init_module(void){
    
    printk("dentro my_sc: in init_module\n");

    //inizializzazione tagLocks
    initTagLocks();

    int id = tag_get(1,CREATE,NO_PERMISSION);
    int id1 = tag_get(1,CREATE,PERMISSION);
    int id2 = tag_get(0,CREATE,PERMISSION);
    int id3 = tag_get(0,CREATE,PERMISSION);
    printk("dentro my_sc: in init_module -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);

    int retSend = tag_send(id, 31, "ciao", 5);
    printk("dentro my_sc: retSend= %d\n", retSend);

    char* buffer = (char*) kzalloc (sizeof(char)*6, GFP_KERNEL);
    printk("dentro my_sc: buffer = %d\n", buffer);
    int retRcv = tag_receive(id, 31, buffer, 6);

    printk("dentro my_sc: retRcv= %d\n", retRcv);


    removeTag(3);
    //printArray();
    int id4 = tag_get(0,CREATE,PERMISSION);
    printArray();
    removeTag(2);
    //printArray();
    int id5 = tag_get(0,CREATE,PERMISSION);
    printArray();
    removeTag(1);
    //printArray();
    int id6 = tag_get(0,CREATE,PERMISSION);
    
    int id7 = tag_get(0,CREATE,PERMISSION);

    removeTag(4);
    int id8 = tag_get(0,CREATE,PERMISSION);
    printArray();
       


    return 0; 
    
}



void cleanup_module(void){
    printk("CLEANUP!\n");
    
}
*/
