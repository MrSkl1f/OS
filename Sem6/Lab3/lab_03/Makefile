obj-m += md.o md1.o md2.o md3.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

test:
	sudo dmesg -C
	sudo insmod md1.ko
	sudo insmod md2.ko
	-sudo insmod md3.ko
	-sudo rmmod md3.ko
	sudo rmmod md2.ko
	sudo rmmod md1.ko
	sudo dmesg

# Загружаемые модули ядра и их взаимодействие