CFLAGS := -Wall -g

default: 6809

6809.o: 6809.c
	$(CC) $(CFLAGS) -c 6809.c

monitor.o: monitor.c
	$(CC) $(CFLAGS) -c monitor.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

ch375.o: ch375.c
	$(CC) $(CFLAGS) -c ch375.c

6809: 6809.o monitor.o main.o ch375.o
	$(CC) $(CFLAGS) -o 6809 main.o monitor.o 6809.o ch375.o -lreadline

clean:
	rm -f 6809 *.o debug.out
