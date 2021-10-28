obj-m += proj.o
proj-objs += usctm.o systemCalls.o ./lib/vtpmo.o ./lib/tagService.o ./lib/utils.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

