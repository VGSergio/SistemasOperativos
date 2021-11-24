# Adelaida
CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=-lreadline -Incurses #activar si se usa readline
 
SOURCES=nivelA.c nivelB.c
LIBRARIES= #.o
INCLUDES= #.h
PROGRAMS=nivelA nivelB
OBJS=$(SOURCES:.c=.o)
 
all: $(OBJS) $(PROGRAMS)
 
#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@
 
nivelA: nivelA.o
	$(CC) $@.o -o $@ $(LIBRARIES)

nivelB: nivelB.o
	$(CC) $@.o -o $@ $(LIBRARIES)
 
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<
 
.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)