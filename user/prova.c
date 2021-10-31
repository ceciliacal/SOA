#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int *tagServiceArray[10];

int** getTagServiceArray(){
    return tagServiceArray;
}


int main(){

    printf("tagServiceArray=%d\n",tagServiceArray);
    char* k = "ciao come va?\nbene tu?\0 ciaooo  ";
    printf("strlen: %d\n",strlen(k));

    for(int i;i<10;i++){
        tagServiceArray[i]=i;
    }

    printf("tagServiceArray[0]=%d\n",tagServiceArray[0]);

    printf("gettagServiceArray=%d\n",getTagServiceArray());

    int* ciao;
    ciao = getTagServiceArray();
    printf("ciao=%d\n",ciao);



    
}





