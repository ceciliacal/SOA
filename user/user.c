#include "../include/const.h"
#include "user.h"




// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a myThreadFun\n", *myid);
    
    int id = syscall(get,0,CREATE,NO_PERMISSION);
  
    printf("Creato tag con ID %d dal thread %d\n",id,*myid);
    pthread_exit(NULL);
}

// The function to remove tag2
void *myThread_remove2(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a remove2\n", *myid);
    
    int res = syscall(ctl,2,REMOVE);
  
    printf("Thread ID: %d, rimosso TAG con ID 2, res = %d\n", *myid, res);
    pthread_exit(NULL);
}

// The function to remove tag4
void *myThread_remove4(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a remove4\n", *myid);
    
    int res = syscall(ctl,4,REMOVE);
  
    printf("Thread ID: %d, rimosso TAG con ID 4, res = %d\n", *myid, res);
    pthread_exit(NULL);
}


int main(int argc, char** argv){
	//syscall(134,1,2);

    int i;
    pthread_t tid;

    struct tag_get_args_t *info  = malloc(sizeof(tag_get_args_t));
    /*
    qui bisognerebbe fare array di struct "info" 
    e poi passarle come parametro (..,..,..,info) nel for
    */

  
    for (i = 0; i < 5; i++){
        pthread_create(&tid, NULL, myThreadFun, (void *)&tid);
        printf("dopo for i=%d\n", i);
    }

    printf("FINE FOR!!!!!!!!\n");
    printf("\n\n inizio sleep...\n");
    sleep(5);
    
    printf("\n\n fine sleep...\n generazione dei thread per rimozione!\n");
    if (pthread_create(&tid, NULL, myThread_remove2, (void *)&tid)!=0){
        printf("ERRORE: fallito thread per remove2\n");   
    };
    if (pthread_create(&tid, NULL, myThread_remove4, (void *)&tid)!=0){
        printf("ERRORE: fallito thread per remove4\n");   
    };
    
    printf("\n\n thread per rimozione creati\n"); 
    pthread_join(tid,0);

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


  

