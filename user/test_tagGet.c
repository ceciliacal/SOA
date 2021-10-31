#include "user.h"


int main(int argc, char** argv){


    //TEST1: creating IPC PRIVATE tag
    int id1 = syscall(get,0,CREATE,NO_PERMISSION);
    if (id1!=-1){
        printf("TEST1: passed\n");
    }

    //TEST2: command is neither create or open
    int id2 = syscall(get,0,3,NO_PERMISSION);
    if (id2==-1){
        printf("TEST2: passed\n");
    }

    //TEST3: opening an IPC_PRIVATE tag
    int id3 = syscall(get,0,OPEN,NO_PERMISSION);
    if (id3==-1){
        printf("TEST3: passed\n");
    }

    //TEST4: open of non existing tag
    int id4 = syscall(get,1,OPEN,PERMISSION);
    if (id4==-1){
        printf("TEST4: passed\n");
    }

    //TEST5: creating tag with permission
    int id5 = syscall(get,2,CREATE,PERMISSION);
    if (id5!=-1){
        printf("TEST5: passed\n");
    }

    //TEST6: creating tag with already existing key
    int id6 = syscall(get,2,CREATE,PERMISSION);
    if (id6==-1){
        printf("TEST6: passed\n");
    }
    



    
    return 0;

}


  

