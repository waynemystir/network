CC=gcc
CFLAGS=-c -Wall

SOURCES=network_utils.c

OBJECTS=$(SOURCES:.c=.o)

all: network.o

network.o :
	$(CC) $(CFLAGS) $(SOURCES) -o $@

clean:
	rm network.o