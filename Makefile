CC=gcc

CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

ALL= server client game

all: $(ALL)

server : server.o utils_v1.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o
server.o: server.c message.h
	$(CC) $(CCFLAGS) -c server.c

client : client.o utils_v1.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o
client.o: client.c message.h
	$(CC) $(CCFLAGS) -c client.c

game : game.o utils_v1.o 
	$(CC) $(CCFLAGS) -o game game.o utils_v1.o 

game.o: game.c message.h utils_v1.h
	$(CC) $(CCFLAGS) -c game.c 

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c

clean:
	rm -f *.o
	rm -f $(ALL)		