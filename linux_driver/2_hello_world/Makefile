KERNELDIR := /home/yyh/ti-sdk-am335x-evm-06.00.00.00/board-support/linux-3.2.0-psp04.06.00.11
CURRENT_PATH := $(shell pwd)
obj-m := hello_device.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean