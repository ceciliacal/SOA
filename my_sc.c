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

    if (command!=CREATE && command!=OPEN||permission!=PERMISSION && permission!=NO_PERMISSION){
        printk("in tag_get: il valore di COMMAND e PERMISSION deve essere 0 o 1.\n");
        return -1;
    }

    printk("Dentro get_tag\n Inizializzazione del servizio...\n");
    
    int tagId;
    
    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    if (command=CREATE){    //create

        pid_t pid = current->pid;
        tagId = addTag(key, uid, pid, permission);
        printk("in tag_get: CREATE -> tagId = %d\n",tagId);

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
*/


int tag_send(int tag, int level, char* buffer, size_t size){

    char* msg;
    int res;

    int checkSize = checkBufferSize(size);
    if (checkSize==-1){
        printk("ERROR: msg size exceeded maximum lenght");
        return -1;
    }

    printk("buffer. buffer =%s\n",buffer);
    msg = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);
    //copy_from_user(msg, buffer, size);
    strcpy(msg, buffer);
    printk("msg allocato. msg =%s\n",msg);

    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    res = deliverMsg(tag, msg, level, size, uid);

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
    }

    buffer = (char*) kzalloc(sizeof(char)*size, GFP_KERNEL);

    const struct cred *cred = current_cred();
    kuid_t uid = cred->uid;

    /*
    perche dovrei contare num di thread che stanno nella wait queue??
    forse per fare check finale che effettivamente tutti i thread sono
    stati svegliati e che hanno ricevuto il messaggio
    */

   tag_t* currTag; //get tag from ID
   int preCheck;

    currTag = getTagFromID(tag);
    preCheck = checkCorrectCondition(currTag, uid);

    if (preCheck == -1){
        return -1;
    }


    printk("dentro tag_receive: recuperato tag con ID %d a indirizzo %d\n",tag,currTag);

    level_t** tagLevels= currTag->levels;

    //faccio la sleep ---> aspetto che arrivi msg
    //uso wait event interruptible(wq, condition)
    res = wait_event_interruptible(*tagLevels[level-1]->waitingThreads,tagLevels[level-1]->msg!=NULL);
    
    if (res == -ERESTARTSYS){
        printk("dentro tag_receive: receiver thread was interrupted by a signal");
        return -1;
    }
    //recupero msg (cioè cosa faccio dopo che thread è stato svegliato) da field
    //del livello

    strcpy(tagLevels[level-1]->msg,buffer);
    printk("dentro deliverMsg: messaggio ricevuto dal thread %d è: %s\n",uid.val, buffer);
    //copy to user

    //if thread si è svegliato per send o per segnale posix

    return 0;

}




int init_module(void){
    
    printk("dentro my_sc: in init_module\n");
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
    return 0; 
    
}



void cleanup_module(void){
    printk("CLEANUP!\n");
    
}
