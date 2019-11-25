obj-m += MorseCode.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) UserProgram.c -o MorseUserProgram

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm MorseUserProgram