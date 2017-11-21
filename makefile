CC = gcc
CFLAGS = -g -Wall


all: mysh test

mysh: mysh.o #process.o variable.o#process.o variable.o
	$(CC) $(CFLAGS) -o mysh mysh.o 

mysh.o: mysh.c process.c variable.c
	$(CC) $(CFLAGS) -c mysh.c process.c variable.c

process.o: process.c
	$(CC) $(CLFAGS) -c -o process.o process.c

variable.o: variable.c
	$(CC) $(CLFAGS) -c -o variable.o variable.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

clean:
	rm -fg *.o mysh process
