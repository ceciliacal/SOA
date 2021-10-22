#include "./lib/initStruct.h"


MODULE_LICENSE("GPL");

/*
poi fare servizio bloccante x thread che provano ad accedere a stesso tag
in stesso momento!

*/

int init_module(void){
    
    
    printk("DENTRO MY_SC\n");

    return 0;  
    
}



void cleanup_module(void){
    printk("CLEANUP!\n");
    
}
