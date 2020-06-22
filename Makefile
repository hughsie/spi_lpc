spi_lpc-y := spi_lpc_main.o bios_data_access.o low_level_access.o
obj-m += spi_lpc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
