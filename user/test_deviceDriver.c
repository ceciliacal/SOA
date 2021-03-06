#include "user.h"
#include "../include/const.h"

char buff[4096];

void * the_thread(void* path){

	char* device;
	int fd;

	device = (char*)path;
	sleep(1);

	printf("opening device %s\n",device);
	fd = open(device,O_RDONLY);
	if(fd == -1) {
		printf("open error on device %s\n",device);
		return NULL;
	}
	printf("device %s successfully opened\n",device);
	
    char* myBuff = malloc(sizeof(char)*4096);
    
	read(fd, myBuff,4095);
	return NULL;

}
int main(int argc, char** argv){

    int ret;
    int major;
    char *path;
    pthread_t tid;

    if(argc<3){
        printf("useg: prog pathname major\n");
        return -1;
    }

    int id = syscall(get,0,CREATE,NO_PERMISSION);
    char* bufferRcv = malloc(sizeof(char)*10);
    

    path = argv[1];
    major = strtol(argv[2],NULL,10);
    printf("creating device %s with major %d\n",path,major);


    sprintf(buff,"%s",path);
    pthread_create(&tid,NULL,the_thread,strdup(buff));
    

    pause();
    return 0;

}