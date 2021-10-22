#include "../include/const.h"
#include "user.h"



// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
    int *myid = (int *)vargp;
    printf("sono thread %d e sto dentro a myThreadFun\n", *myid);
    
    int id = syscall(134,2,CREATE,NO_PERMISSION);
  
    printf("Thread ID: %d, TAG ID: %d\n", *myid, id);
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
        

    /*
    int id = syscall(134,1,CREATE,NO_PERMISSION);
    int id1 = syscall(134,1,CREATE,PERMISSION);
    int id2 = syscall(134,0,CREATE,PERMISSION);
    int id3 = syscall(134,0,CREATE,PERMISSION);

    printf("dentro user: -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);
    */

    return 0;

}


  

