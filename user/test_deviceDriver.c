#include "user.h"

int i;
char buff[4096];
#define DATA "ciao a tutti\n"
#define SIZE strlen(DATA)

void * the_thread(void* path){

	char* device;
	int fd;

	device = (char*)path;
	sleep(1);

	printf("opening device %s\n",device);
	fd = open(device,O_RDWR);
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

    path = argv[1];
    major = strtol(argv[2],NULL,10);
    printf("creating device %s with major %d\n",path,major);

   
    sprintf(buff,"mknod %s c %d 0\n",path,major);
    system(buff);

    sprintf(buff,"%s",path);
    pthread_create(&tid,NULL,the_thread,strdup(buff));
    

    pause();
    return 0;

}