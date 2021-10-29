#include "../include/const.h"
#include "user.h"


int main(int argc, char** argv){

    int id = syscall(get,3,OPEN,NO_PERMISSION);
    printf("id=%d\n",id);

    return 0;

}


  

