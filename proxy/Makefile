all: a.out

a.out: 1.o
	gcc 1.o -o a.out -lm -g

1.o: 1.c
	gcc -c 1.c -o 1.o -lm -g

clear:
	rm *.o *.out

leha: 
	gcc leha.c -lm -g -o a.out

.PHONY: all clear
