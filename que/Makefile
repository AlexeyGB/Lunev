CC = gcc
CFLAGS = -Wall -Werror -o2
LFLAGS = -lgcov --coverage
SOURCES = 1.c 2.c 
OBJECTS = $(SOURCES:.c=.o)
HEADERS = 1.h
EXCECUTABLE = a.out

all: $(EXCECUTABLE)

$(EXCECUTABLE): $(OBJECTS) 
	$(CC) $^  -o $@

$(OBJECTS): $(HEADERS)

.c: .o
	$(CC) $^  -o $@

gcov: 
	gcc $(SOURCES) -fprofile-arcs -ftest-coverage -o $(EXCECUTABLE)

clear: 
	rm $(OBJECTS) $(EXCECUTABLE)

.PHONY: all clear gcov