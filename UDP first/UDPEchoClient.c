#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ECHOMAX 255 /*Longest string to echo*/

void DieWithError(char *errorMessage);

int main(int argc, char *argv[])
{
	int sock; /*Socket descriptor*/
	struct sockaddr_in echoServAddr; /* Echo server address */
	struct sockaddr_in fromAddr; /* Source address of echo */
	unsigned short echoServPort;
	unsigned int fromSize;
	char *servIP;
	char *echoString;
	char echoBuffer[ECHOMAX+1];
	unsigned int echoStringLen;
	int respStringLen;

	if ((argc<3)||(argc>4))
	{
			fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n"), argv[0];
			exit(1);
	}

	servIP = argv[1];
	echoString = argv[2];
	
	if ((echoStringLen = strlen(echoString)) > ECHOMAX) /* Check input length */
		DieWithError("Echo word too long");

	if(argc == 4)
		echoServPort = atoi(argv[3]);
	else
		echoServPort = 7;

	/* Create a datagram/UDP socket */
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET;   /* Internet addr family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort);  /* Server port */

	
	/*send the string to the server*/
	if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))!=echoStringLen)
		DieWithError("send() sent a different number of bytes than expected");

	/*Receive a response*/
	fromSize = sizeof(fromAddr);
	if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) != echoStringLen)
		DieWithError("recvfrom() failed") ;
	
	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}
	
	
	/* null-terminate the received data */
	echoBuffer[respStringLen] = '\0' ;
	printf("Received: %s\n", echoBuffer); /* Print the echoed arg */
	close(sock);
	exit(0);
}