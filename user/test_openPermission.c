#include "user.h"

/*
Before running this file it must be executed
first "test_tagGet.c and then it has to be changed User Id.
*/


int main(int argc, char** argv){

    int key=2;

    //TEST1: opening tag with permission after changing user
    int id1 = syscall(get,2,OPEN,NO_PERMISSION);  
    if (id1==-1){
        printf("TEST1: passed - permission to get TAG with key %d denied\n",key);
    }

    //TEST2: opening tag with permission after changing user
    int id2 = syscall(get,2,OPEN,PERMISSION);
    if (id2==-1){
        printf("TEST2: passed - permission to get TAG with key %d denied\n",key);
    }
    
    return 0;

}


  

