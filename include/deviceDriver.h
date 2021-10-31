#define DEVICE_NAME "mychardev"

//Declaration of deviceDriver functions
static int devOpen (struct inode *, struct file *);
static ssize_t devRead (struct file *, char *, size_t, loff_t *);
static ssize_t devWrite (struct file *, const char *, size_t, loff_t *);
static int devRelease (struct inode *, struct file *);

int register_device(void);
void unregister_device(void);
