#all:	a.out

a.out: 1.o 2.o
	gcc -o a.out 1.o 2.o

1.o: 1.c
	gcc -c -o 1.o 1.c

2.o: 2.c
	gcc -c -o 2.o 2.c

1.o 2.o: 1.h

#clean:	-m -rf *.o

#.PHONY: all clean
