CC = gcc
CFLAGS = -g -Wall -Werror -o2
LFLAGS = -lgcov --coverage
SOURCES = 1.c 
OBJECTS = $(SOURCES:.c=.o)
HEADERS = 
EXCECUTABLE = a.out

all: $(EXCECUTABLE)

$(EXCECUTABLE): $(OBJECTS) 
	$(CC) $^  -g -lm -o  $@

$(OBJECTS): $(HEADERS)

.c: .o
	$(CC) $^  -g -o   $@

gcov: 
	gcc $(SOURCES) -g -fprofile-arcs -ftest-coverage -o $(EXCECUTABLE)

clear: 
	rm *.out *.gcov *.gcda *.gcno

.PHONY: all clear gcov
