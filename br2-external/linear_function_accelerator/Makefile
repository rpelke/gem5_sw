obj-m += lfa.o
lfa-y := src/lfa_main.o src/lfa_platform.o src/lfa_device.o

ccflags-y=-I$(PWD)/inc

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
