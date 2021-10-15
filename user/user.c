
#include <unistd.h>
#include <stdio.h>
#include "../include/const.h"


int main(int argc, char** argv){
	//syscall(134,1,2);
    int id = syscall(134,1,CREATE,NO_PERMISSION);
    int id1 = syscall(134,1,CREATE,PERMISSION);
    int id2 = syscall(134,0,CREATE,PERMISSION);
    int id3 = syscall(134,0,CREATE,PERMISSION);

    printf("dentro user: -> id=%d  id1=%d  id2=%d  id3=%d\n",id,id1,id2,id3);

    return 0;

}