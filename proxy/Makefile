all: a.out

a.out: 1.o
	gcc 1.o -o a.out -lm

1.o: 1.c
	gcc -c 1.c -o 1.o -lm

clear:
	rm *.o *.out

.PHONY: all clear
