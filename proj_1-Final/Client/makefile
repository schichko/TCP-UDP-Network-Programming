CC=gcc
CFLAGS=-Wall

all: client server

OTHER_OBSS = DieWithError.o HandleTCPClient.o

client : client.o $(OTHER_OBSS)

server : server.o $(OTHER_OBSS)

DieWithError.o : DieWithError.c
	$(CC) -c DieWithError.c

HandleTCPClient.o : HandleTCPClient.c
	$(CC) -c HandleTCPClient.c

client.o: client.c
	$(CC) -c client.c

server.o: server.c 
	$(CC) -c server.c

clean:
	rm -f	client.o DieWithError.o HandleTCPClient.o server.o client.exe server.exe
				
