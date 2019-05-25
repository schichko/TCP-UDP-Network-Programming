#include <stdio.h> /* for printf() and fprintf() */
#include <stdio.h> /* for printf() and fprintf() */
#include <unistd.h> /* for close() */

#define RCVBUFSIZE 32 /* Size of receive buffer */

void DieWithError(char *errorMessage);

void HandleTCPClient(int clntSocket)
{
	char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
	int recvMsgSize;

	printf("Enter HandleTCPClient, clntSocket: %d\n", clntSocket);
	
	
	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed") ;
	
	printf("recvMsgSize: %d\n", recvMsgSize);
			

	/* Send received string and receive again until end of transmission */
	while (recvMsgSize > 0) /* zero indicates end of transmission */
	{
		/* Echo message back to client */
		if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
			DieWithError("send() failed");
		
		echoBuffer[recvMsgSize] = '\0';
		printf("Received: ");
		printf(echoBuffer);

		/* See if there is more data to receive */
		if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
			DieWithError("recv() failed") ;

	}

	close(clntSocket); /* Close client socket */
}