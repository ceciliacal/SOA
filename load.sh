echo "running command: sudo insmod proj.ko"
sudo insmod proj.ko
echo "running command: sudo mknod /dev/mychardev c 237 0"
sudo mknod /dev/mychardev c 237 0
