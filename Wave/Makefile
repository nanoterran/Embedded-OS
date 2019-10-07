TARGET = Wave
CC = gcc

OBJS += Wave.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

.PHONY: clean
clean:
	rm *.o
	rm $(TARGET)
	rm *.dat