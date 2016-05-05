CC = gcc
CFLAGS = -g -Wall -Werror -o2
LFLAGS = -lgcov --coverage
SOURCES = final.c  
OBJECTS = $(SOURCES:.c=.o)
HEADERS = 
EXCECUTABLE = a.out

all: $(EXCECUTABLE)

$(EXCECUTABLE): $(OBJECTS) 
	$(CC) $^  -g  -lm -pthread -o  $@

$(OBJECTS): $(HEADERS)

.c: .o
	$(CC) $^  -g -pthread -o   $@

gcov: 
	gcc $(SOURCES) -g -lm -pthread -fprofile-arcs -ftest-coverage -o $(EXCECUTABLE)

clear: 
	rm *.out *.gcov *.gcda *.gcno *.o

.PHONY: all clear gcov
