obj-m += the_sc.o
the_sc-objs += my_sc.o ./lib/initStruct.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

