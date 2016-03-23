all:
	gcc -c uthread.c

main: uthread.c main.c
	gcc -o main uthread.c main.c
