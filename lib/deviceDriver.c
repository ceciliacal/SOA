#include "tagService.h"
#include "deviceDriver.h"

MODULE_LICENSE("GPL");

static struct cdev cdev;
static struct class devClass;
static int major;

int maxSizeLine = 32;   //1 line of device file should be 20 bytes
//1 line = 20 bytes -> 4bytes*4field (key,uid.val,level,nThreads) + 3bytes for space(char) + 1byte(\n)


//dev_t sarebbe MKDEV

//Declaration of deviceDriver functions
static int devOpen (struct inode *, struct file *);
static ssize_t devRead (struct file *, char __user *, size_t, loff_t *);
static ssize_t devWrite (struct file *, const char __user *, size_t, loff_t *);
static int devRelease (struct inode *, struct file *);

static struct file_operations fileOps = {
    .owner =    THIS_MODULE;
    .read  =   devRead;
    .write =    devWrite;
    .open  =     devOpen;
    .release =  devRelease;
}

/*
int (*flush) (struct file *, fl_owner_t id);
loff_t (*llseek) (struct file *, loff_t, int);
long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);  


dentro a init module va chiamata funzione register_chrdev (che va implementata qui)
e poi nella exit va chiamata unregister che fa la free del major number
*/

static int devOpen (struct inode *, struct file *){
    //serve per inizializzare variabili del driver
    //success
    return 0;
}

static int devRelease (struct inode *, struct file *){
    //free delle variabili del driver e della memoria usata
    return 0;
}

ssize_t devWrite (struct file *, const char __user *, size_t, loff_t *off){
    printk("device driver is read only\n");
    return -1;
}

char* myAppend(char* dest, char* src){

    // Determine new size
    int newSize = strlen(dest)  + strlen(src) + 1; 

    // Allocate new buffer
    char* newBuffer = malloc(newSize*sizeof(char));

    // do the copy and concat
    strcat(newBuffer,dest);
    strcat(newBuffer,src);

    return newBuffer;
}

static ssize_t devRead (struct file *filp, char __user *buf, size_t size, loff_t *off){

    int i, j, devBufferSize;
    char* header, devBuffer;

    printk("%s: somebody called a read on dev with major number %d\n",MODNAME,get_major(filp));


    if(size<0 || buf==NULL || off<0){
        printk(KERN_ERR"%s: size cannot be negative\n",MODNAME);
        return -1;
    }

    if(size==0){
        return 0;
    }

    tag_t* tags = getTagServiceArray();
    rwlock_t tagLocks[MAX_N_TAGS] = getTagLocks();

    //header - 64 bytes
    header = kzalloc(sizeof(char)*maxSizeLine,GFP_KERNEL);
    devBuffer = header;

    if(header==NULL){
        printk(KERN_ERR"%s: Error in buffer allocation\n",MODNAME);
    }
    sprintf("%s\n","key uid level threads");

    for (i=0;i<MAX_N_TAGS;i++){

        read_lock(&tagLocks[i]);

        if (tags[i]!=NULL){
            
            for (j=0;j<N_LEVELS;j++){

                spin_lock(&tags[i]->levelLocks[j]);
                level_t* currLevel = tags[i]->levels[j];
                
                //check if there is any waiting thread on that level
                if(currLevel->numThreadsWq>0){

                    char* tempLine = kzalloc*(sizeof(char)*maxSizeLine,GFP_KERNEL);
                    sprintf(tempLine,"%-20d %-20d %-20d %-20d\n",tags[i]->key,tags[i]->creatorUserId.val,currLevel->number,currLevel->numThreadsWq);

                    char* line = myAppend(devBuffer,tempLine);

                    kfree(tempLine);
                    
                    if (line==NULL){
                        printk(KERN_ERR"%s: Error in buffer allocation\n",MODNAME);
                        spin_unlock(&tags[i]->levelLocks[j]);
                        read_unlock(&tagLocks[i]);
                        return -1;
                    }

                    devBuffer = line;
                }
                
                spin_unlock(&tags[i]->levelLocks[j]);
            }
        }
        read_unlock(&tagLocks[i]);
    }

    devBufferSize = strlen(devBuffer);
    printk(KERN_INFO"%s: devBufferSize=%d\n",MODNAME,devBufferSize);
    
    if((devBufferSize - *off) < len) len = devBufferSize - *off;
    ret = copy_to_user(buff,&(devBuffer),len);

    *off += (len - ret);

    return len - ret;

}



void unregister_device(void){

    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "%s: mychardev unregistered, it was assigned major number %d\n", MODNAME,major);

}

int register_device(void){
    
    major = register_chrdev(0, DEVICE_NAME, &fops);

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





