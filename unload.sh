echo "running command: sudo rmmod proj"
sudo rmmod proj 
echo "running command: rm /dev/mychardev"
sudo rm /dev/mychardev
echo "running command: cleaning enviroment"
sudo find . -type f \( -iname \*.cmd -o -iname \*.symvers -o -iname \*.order -o -iname \*.ko -o -iname \*.o -o -iname \*.mod -o -iname \*.mod.c -o -iname \*.o.d \) -delete
