#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int *tagServiceArray[10];

int** getTagServiceArray(){
    return tagServiceArray;
}

char* myAppend(char* dest, char* src){
    // real[i].buffer += buffer; 

    // Determine new size
    int newSize = strlen(dest)  + strlen(src) + 1; 

    // Allocate new buffer
    char* newBuffer = malloc(newSize*sizeof(char));

    // do the copy and concat
    strcat(newBuffer,dest);
    strcat(newBuffer,src);

    // release old buffer
    //free(dest);
    //free(src);

    return newBuffer;
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

    char* s0=malloc(sizeof(char)*1);;
    char* s1="cacca\n";
    char* s2="merda\n";
    int newSize = strlen(s1)+strlen(s2);
    char* s3 = malloc(sizeof(char)*newSize);
    strcat(s3,s1);
    strcat(s3,s2);
    printf("%s\n",s3);

    char* r= myAppend(s0,s1);
    printf("myAppend: \n%s\n",r);

    char* ress= myAppend(r,s2);
    printf("myAppend: \n%s\n",ress);
    s3="aiuto\n";
    char* ress2= myAppend(ress,s3);
    printf("myAppend: \n%s\n",ress2);


    
}





