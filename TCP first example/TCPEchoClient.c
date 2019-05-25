


#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RCVBUFSIZE 32 /*Size of receive buffer*/

void DieWithError(char *errorMessage);

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr;
	unsigned short echoServPort;
	char *servIP;
	char *echoString;
	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen;
	int bytesRcvd, totalBytesRcvd;

	if ((argc<3)||(argc>4))
	{
			fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n"), argv[0];
			exit(1);
	}

	servIP = argv[1];
	echoString = argv[2];

	if(argc == 4)
		echoServPort = atoi(argv[3]);
	else
		echoServPort = 7;

	/*Create a socket using TCP*/
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(echoServPort);

	/*Establish connection to echo server*/
	if (connect(sock, (struct scokaddr*) &echoServAddr, sizeof (echoServAddr))<0)
		DieWithError("connect( ) failed");

	printf("connect succeed\n");
	echoStringLen = strlen(echoString);

	/*send the string*/
	if (send(sock, echoString, echoStringLen, 0)!=echoStringLen)
		DieWithError("send() sent a different number of bytes than expected");

	printf("send succeed\n");
	/*Receive the same string back from the server*/
	totalBytesRcvd=0;
	
	while (totalBytesRcvd < echoStringLen)
	{
		/**/

		if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE-1, 0))<=0)
			DieWithError("recv() failed or connection closed prematurely");
		
		printf("Received: bytesRcvd=%d\n", bytesRcvd);
		totalBytesRcvd += bytesRcvd;
		echoBuffer[bytesRcvd] = '\0';
		printf(echoBuffer);
	}

	printf("\n");

	close(sock);
	exit(0);
}