obj-m += MorseCode.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) UserProgram.c -o MorseUserProgram
	@echo none > /sys/class/leds/beaglebone\:green\:usr0/trigger

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm MorseUserProgram
	@echo heartbeat > /sys/class/leds/beaglebone\:green\:usr0/trigger