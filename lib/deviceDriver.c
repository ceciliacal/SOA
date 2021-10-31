#include "../include/tagService.h"
#include "../include/deviceDriver.h"

MODULE_LICENSE("GPL");

static int major;

int maxSizeLine = 100;   //1 line of device file should be 20 bytes
//1 line = 20 bytes -> 4bytes*4field (key,uid.val,level,nThreads) + 3bytes for space(char) + 1byte(\n)


static struct file_operations fileOps = {
    .owner =    THIS_MODULE,
    .read  =    devRead,
    .write =    devWrite,
    .open  =    devOpen,
    .release =  devRelease
};


static int devOpen (struct inode *inode, struct file *file){
    //serve per inizializzare variabili del driver
    return 0;
}

static int devRelease (struct inode *inode, struct file *file){
    //free delle variabili del driver e della memoria usata
    return 0;
}

ssize_t devWrite (struct file *filp, const char *buff, size_t len, loff_t *off){
    printk(KERN_ERR"%s:Device %s is read only\n",MODNAME,DEVICE_NAME);
    return -1;
}

char* myAppend(char* dest, char* src){

    // Determine new size
    int newSize = strlen(dest) + strlen(src) + 1; 
    printk(KERN_INFO"%s: newSize is %d\n",MODNAME, newSize);
    
    // Allocate new buffer
    char* newBuffer = kzalloc(newSize*sizeof(char), GFP_KERNEL);
    if (newBuffer==NULL){
        printk(KERN_ERR"%s: Error in new line buffer allocation\n",MODNAME);
        return NULL;
    }
    
    // do the copy and concat
    strcpy(newBuffer,dest);
    strcat(newBuffer,src);

    printk(KERN_INFO "%s: buffer is: %s\n",MODNAME,newBuffer);
    
    return newBuffer;
}


static ssize_t devRead (struct file *filp, char *buff, size_t len, loff_t *off){

    int i, j, devBufferSize, ret;
    char* header;
    char* devBuffer;
    char* newBuffer;

    printk("%s: sto in devRead!!!!!!!\n",MODNAME);


    if(len<0 || buff==NULL || off<0){
        printk(KERN_ERR"%s: size cannot be negative\n",MODNAME);
        return -1;
    }
    printk("%s: dopo 1 if!!!!!!!\n",MODNAME);

    if(len==0){
        return 0;
    }
    printk("%s: dopo 2 if!!!!!!!\n",MODNAME);

    tag_t **tags;
    tags = getTagServiceArray();
    
    //rwlock_t tagLocks = getTagLocks();
    rwlock_t *tagLocks;
    tagLocks = getTagLocks();

    printk("%s: dopo array di merdaaaaaaaaaaa\n",MODNAME);

    //header - 64 bytes
    header = "key uid level threads\n";
    newBuffer = header;


    printk("%s: dopo header\n",MODNAME);

    for (i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);

        if (tags[i]!= NULL){
            
            for (j=0;j<N_LEVELS;j++){

                spin_lock(&tags[i]->levelLocks[j]);
                level_t* currLevel = tags[i]->levels[j];
                printk("%s: currLevel num=%d\n",MODNAME,currLevel->number);
                
                //check if there is any waiting thread on that level
                if(currLevel->numThreadsWq > 0 ){

                    char* tempLine = kzalloc(sizeof(char)*maxSizeLine,GFP_KERNEL);
                    
                    if (tempLine==NULL){
                        printk(KERN_ERR"%s: Error in buffer allocation\n",MODNAME);
                        spin_unlock(&tags[i]->levelLocks[j]);
                        read_unlock(&tagLocks[i]);
                        return -1;
                    }

                    int h = sprintf(tempLine,"%-5d %-5d %-5d %-5d\n",tags[i]->key,tags[i]->creatorUserId.val,currLevel->number,currLevel->numThreadsWq);
                    
                    printk("res linea h= %d\n",h);
                    devBuffer = myAppend(newBuffer,tempLine);

                    kfree(tempLine);
                    newBuffer = devBuffer;
                
                }

                spin_unlock(&tags[i]->levelLocks[j]);
                
            }
        }
        read_unlock(&tagLocks[i]);
    }

    printk("%s: CACA4\n",MODNAME);
    devBufferSize = strlen(devBuffer);
    printk(KERN_INFO"%s: devBufferSize=%d\n",MODNAME,devBufferSize);

    if((devBufferSize - *off) < len) len = devBufferSize - *off;
    ret = copy_to_user(buff,&(devBuffer[*off]),len);

    *off += (len - ret);

    return len - ret;

}



void unregister_device(void){

    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "%s: mychardev unregistered, it was assigned major number %d\n", MODNAME,major);

}

int register_device(void){
    
    major = register_chrdev(0, DEVICE_NAME, &fileOps);

	if (major < 0) {
	  printk(KERN_ERR"%s: Cannot obtain major number\n",MODNAME);
	  return major;
	}

	printk(KERN_INFO "%s: mychardev registered, it is assigned major number %d\n", MODNAME,major);


    /*
    dev_t dev;

    if(alloc_chrdev_region(&dev, 0, 1, "mychardev")<0){
        printk(KERN_ERR"%s: Error in char device region allocation", MODNAME);
        return -1;
    }

    major = MAJOR(dev);
    printk("%s: major=%d", MODNAME,major);

    chrdevClass = lass_create(THIS_MODULE, "mychardev");
    if (chrdevClass)==NULL){
        printk(KERN_ERR"%s: Error in class creation", MODNAME);
        return -1;
    }

     
    // init new device
    cdev_init(&cdev, &fileOps);
    cdev.owner = THIS_MODULE;

    // add device to the system where "i" is a Minor number of the new device
    cdev_add(&cdev, MKDEV(major, 0), 1);

    // create device node /dev/mychardev-x where "x" is "i", equal to the Minor number
    device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
    */

    return 0;
}





