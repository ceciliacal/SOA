obj-m += the_usctm.o
the_usctm-objs += usctm.o ./lib/vtpmo.o my_sc.o ./lib/initStruct.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

