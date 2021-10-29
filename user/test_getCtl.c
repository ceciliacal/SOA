#include "../include/const.h"
#include "user.h"

int removingId = 2;          //tag ID to remove
int loopCreate = 100;
int loopRmv = 20;
int loopRecreate = 4;
int lastTagId;



void *createTag(void *vargp)
{
    int *myid = (int *)vargp; 

    int id = syscall(get,0,CREATE,NO_PERMISSION);
  
    if (id!=-1){

        printf("CREATE: thread %d created tag with ID %d\n",*myid,id);
    }
    else{
        
        printf("CREATE: ERROR   -   return value is %d\n",id);

    }

    lastTagId=id;

    pthread_exit(NULL);
}

void *removeTags(void *vargp)
{
    int *myid = (int *)vargp;
    
    removingId = removingId+2;

    int res = syscall(ctl,removingId,REMOVE);

    if (res==0){

        printf("REMOVE: thread %d removed tag with ID %d\n",*myid,removingId);

    }
    else{
        
        printf("REMOVE: ERROR   -   return value is %d\n",res);

    }
  
    pthread_exit(NULL);
}


int main(int argc, char** argv){

    int i;
    pthread_t tidC[loopCreate];
    pthread_t tidR[loopRmv];
    pthread_t tidRecreate[loopRecreate];


    printf("Spawning threads to create %d tags... :\n",loopCreate);
    for (i = 0; i < loopCreate; i++){

        pthread_create(&tidC[i], NULL, createTag, (void *)&tidC[i]);
        
    }

    
    sleep(5);

    printf("Spawning threads to remove %d tags... :\n",loopRmv);
    for (i = 0; i < loopRmv; i++){
        
        pthread_create(&tidR[i], NULL, removeTags, (void *)&tidR[i]);

    }


    for (i = 0; i < loopRecreate; i++){
        
        pthread_create(&tidRecreate[i], NULL, createTag, (void *)&tidRecreate[i]);

    }

    //Testing AWAKE_ALL on last tag created
    //We expect it to fail since there are no threads in its waitqueues
    int resAwake = syscall(ctl,lastTagId,AWAKE_ALL);
    if (resAwake==0){
        printf("Syscall tag_ctl (AWAKE_ALL) result: %d  -  ERROR\n", resAwake, id);
    }else if (resAwake==-1){
        printf("Syscall tag_ctl (AWAKE_ALL) result: %d  -  SUCCESS\n", resAwake, id);
    }


        
    for (i = 0; i < loopCreate; i++){
        pthread_join(tidC[i],0);
    }
    for (i = 0; i < loopRmv; i++){
        pthread_join(tidR[i],0);
    }
    for (i = 0; i < loopRecreate; i++){
        pthread_join(tidRecreate[i],0);
    }

    
    return 0;

}


  

