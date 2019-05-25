CC=gcc
CFLAGS=-Wall

all: UDPEchoClient UDPEchoServer

UDPEchoClient :	UDPEchoClient.o	DieWithError.o 

UDPEchoServer :	UDPEchoServer.o	DieWithError.o 

DieWithError.o : DieWithError.c
	$(CC) -c DieWithError.c

UDPEchoClient.o: UDPEchoClient.c
	$(CC) -c UDPEchoClient.c

UDPEchoSever.o: UDPEchoSever.c 
	$(CC) -c UDPEchoSever.c

clean:
	rm -f	UDPEchoClient.o	DieWithError.o UDPEchoServer.o UDPEchoClient.exe UDPEchoServer.exe
				
