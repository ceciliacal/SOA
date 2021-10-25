obj-m += proj.o
proj-objs += usctm.o my_sc.o ./lib/vtpmo.o ./lib/initStruct.o ./lib/utils.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

