#include "../include/const.h"
#include "user.h"

int remId = 2;
int loopCreate = 100;
int loopRmv = 20;


// The function to be executed by all threads
void *createTag(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a myThreadFun\n", *myid);
    
    int id = syscall(get,0,CREATE,NO_PERMISSION);
  
    printf("Creato tag con ID %d dal thread %d\n",id,*myid);
    pthread_exit(NULL);
}

// The function to remove tag2
void *removeTags(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a remove2\n", *myid);
    
    remId = remId+2;

    int res = syscall(ctl,remId,REMOVE);
  
    printf("Thread ID: %d, rimosso TAG con ID %d, res = %d\n", *myid, remId, res);
    pthread_exit(NULL);
}


int main(int argc, char** argv){

    int i;

    struct tag_get_args_t *info  = malloc(sizeof(tag_get_args_t));
    /*
    qui bisognerebbe fare array di struct "info" 
    e poi passarle come parametro (..,..,..,info) nel for
    */

    printf("creazione thread CREATE:\n");
    pthread_t tid1[loopCreate];
    for (i = 0; i < loopCreate; i++){

        pthread_create(&tid1[i], NULL, createTag, (void *)&tid1[i]);
        
    }

    printf("FINE FOR!!!!!!!!\n");
    printf("\n\n inizio sleep...\n");
    sleep(5);
    
    printf("\n\n fine sleep...\n generazione dei thread per rimozione!\n");
    pthread_t tid2[loopRmv];
    for (i = 0; i < loopRmv; i++){
        pthread_create(&tid2[i], NULL, removeTags, (void *)&tid2[i]);
        //printf("dopo for i=%d\n", i);
    }

    printf("\n\n inizio sleep per RICREARE...\n");
    //sleep(5);
    
    printf("\n\n fine sleep...\n generazione dei thread per creazione di altri tag nei posti liberi!\n");


    
    pthread_t tid3[4];
    for (i = 0; i < 4; i++){
        pthread_create(&tid3[i], NULL, createTag, (void *)&tid3[i]);
        //printf("dopo for i=%d\n", i);
    }

    
    printf("\n\n thread per rimozione creati\n"); 
    
    for (i = 0; i < loopCreate; i++){
        pthread_join(tid1[i],0);
        //printf("dopo for i=%d\n", i);
    }
    for (i = 0; i < loopRmv; i++){
        pthread_join(tid2[i],0);
        //printf("dopo for i=%d\n", i);
    }
    for (i = 0; i < 4; i++){
        pthread_join(tid3[i],0);
        //printf("dopo for i=%d\n", i);
    }

    

    /*
    int id = syscall(134,1,CREATE,NO_PERMISSION);
    int id1 = syscall(134,1,CREATE,PERMISSION);
    int id2 = syscall(134,0,CREATE,PERMISSION);
    int id3 = syscall(134,0,CREATE,PERMISSION);

    printf("dentro user: -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);
    */

   //printf("dentro user: -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);

    return 0;

}


  

