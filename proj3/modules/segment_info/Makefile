obj-m := segment_info.o

KDIR := /usr/src/linux-headers-$(shell uname -r)
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean	