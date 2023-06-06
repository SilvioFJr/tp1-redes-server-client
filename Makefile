# all:

# 	gcc -Wall -c common.c
# 	gcc -Wall client.c common.o -o client
# 	gcc -Wall server.c common.o -o server

CC = gcc
CFLAGS = -Wall -Wextra

all: server client

server: server.o common.o
	$(CC) $(CFLAGS) -o server server.o common.o

client: client.o common.o
	$(CC) $(CFLAGS) -o client client.o common.o

common.o: common.c
	$(CC) $(CFLAGS) -c common.c

clean:
	rm -f server client *.o
