ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := spi_lpc.o
8123-y := spi_lpc.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

in: default
	sudo insmod spi_lpc.ko
	sudo cat /sys/kernel/security/spi/bioswe
	sudo cat /sys/kernel/security/spi/ble
	sudo cat /sys/kernel/security/spi/smm_bwp

out:
	sudo rmmod spi_lpc

endif
