#include <unistd.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/

#define RCVBUFSIZE 256 /* Size of receive buffer */
FILE *fptr; 

void DieWithError(char *errorMessage); //For providing an Error messaage

void HandleTCPClient(int clntSocket)
{
	char buff[RCVBUFSIZE];     /* Buffer for echo string */
	int sendMSGSize;	//Used to see how long the message we are sending is

	int sendbytes = 0;	//The 
	int clientChecksum = -1; //The checksum value of the client
	int checksum = -1;	//The servers checksum

	printf("Enter HandleTCPClient, clntSocket: %d\n", clntSocket);
	
	
	/* Receive message from client */
	//Using sendMSGSize here to lessen the amount of variables, is used for sending later
	if ((sendMSGSize = recv(clntSocket, buff, RCVBUFSIZE, 0)) < 0){
		DieWithError("recv() failed") ;
	}
	
	     printf("sendMSGSize: %d\n", sendMSGSize);

         buff[sendMSGSize] = '\0';   //Get rid of new line character
         printf("File name from client: %s\n", buff); 

         if(access(buff, F_OK ) != -1 ) {	//If we can acess the file that the client requested we move in and start reading it into the buffer in segments
             printf("File Exists \n"); 
             // Open file 
             fptr = fopen(buff, "r"); //Opens the file to the bugger
            if (fptr != NULL) {
				 
				 fseek(fptr, 0L, SEEK_END);	//This will let us see how big the file is
				 int sz = ftell(fptr);	//sz becomes the full size of the file
				 int outbytes = 0; 	// used in testing
				 int stop = 0;	//If stop = 0 that means we have more to read, if it =1 we are done with our file
				 printf ("\nTotal Size:%d\n",sz);	//Prints the total file size
				 rewind(fptr);	//Rewinds the file so we can start from the beginning 
				 while(sz > 0 && stop == 0){

					//send(clntSocket,&stop,sizeof(int),0);
					//printf("\nNEW LOOP\n");
					sendMSGSize = fread(buff, sizeof(char), RCVBUFSIZE - 1, fptr);	//We read the buffer and add however much of it we can fit knowing the buffer size, for example if it is more than 256 we can only fit 255 bytes in the segment to account for the null terminator
				
					//printf("sendMSGSize: %d\n",sendMSGSize);
					for(int j = 0;j<sendMSGSize;j++){
						checksum = (checksum + (int)buff[j])%346742;
						//printf("\nLetter:%c Checksum:%d",buff[j],(int)buff[j]);
					}
					
					//printf("checksum Total :%d",checksum);
					//A saftey check for reading the file
					if ( ferror( fptr ) != 0 ) {
						fputs("Error reading file", stderr);
					} else {
						buff[sendMSGSize++] = '\0'; /* Just to be safe. */
					}
					//printf("\nSending: ");
					//printf(buff);
					//printf("\nTest1\n");
					sendbytes = send(clntSocket, buff, sendMSGSize, 0);	//Sends the client the current buffer
					
					//printf("\nTest2\n");
					//printf("  Sendbytes: %d\n",sendbytes);
					if (sendbytes != sendMSGSize){	//A check to see if something went wrong with the send 
						DieWithError("send() failed");
					}
					//outbytes = outbytes +sendbytes;
					//printf("\n\n\nSZ:%d    RECV:%d",sz,sendMSGSize);
					sz = sz - (sendMSGSize - 1); //We decrease the size by the amount we have just sent
					//printf("\nsz:%d\n",sz);
					//printf("HERE\n");
					if(sz <= 0){	//If the size is less than or equal to 0 we know our file has been sent completly and want to send the client the stop message
						printf("Checking for file completion\n");
						printf("Server Checksum: %d\n",checksum);
						sleep(1);
						stop = 1;	//Stop 
						//printf("IFF\n");
						send(clntSocket,&stop,sizeof(int),0);	//Send stop
						//sleep(1);
						send(clntSocket,&checksum,sizeof(int),0);	//Send checksum for client to compare to
					}
					else{
						//printf("ELSE\n");
						send(clntSocket,&stop,sizeof(int),0);	//Let the client know we must keep going
					}
				 }
                 fclose(fptr);	//Close the file
             }
         } 
         else {	//Lets the server know that the file that it requested does not exist
             printf("Requested file does not Exist \n"); 
             //n = 0; 
             // copy server message in the buffer 
             sprintf(buff, "Requested file does not exist\n\0"); 
			 sendMSGSize = strlen(buff);
             printf("CHECK:%d\n",sendMSGSize);
             // and send that buffer to client 
             send(clntSocket, buff, sendMSGSize,0);
         }		

	close(clntSocket); /* Close client socket */
}