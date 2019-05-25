// Write CPP code here 
#include <unistd.h>
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <unistd.h>
#include <string.h> 
#include <sys/socket.h> 

#define RCVBUFSIZE 256/*Size of receive buffer*/
void DieWithError(char *errorMessage);



int main(int argc, char *argv[]) 
{ 
    int sock; /*Socket descriptor*/
	struct sockaddr_in servAddr;
	unsigned short servPort; //Port of the server you want to connect to 
	char *servIP;	//IP of the server you want to connect to 
	char *fileString;	//Name of the file you wish to reciever 
	char buff[RCVBUFSIZE]; //Buffer we will use for file transfer
	unsigned int fileStringLen; //The length of the original file name string
	int bytesRcvd; //Amount of bytes recieved

	if ((argc<3)||(argc>4))	//If this happens we have not given the proper number of arguments 
	{
			fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n"), argv[0];
			exit(1);
	}

	servIP = argv[1];	//The Server IP is set
	fileString = argv[2];	//The file string name is set

    FILE *newfile;	//We create a file with the name of the filestring we were given, if this the file does not exist it is later deleated 
    newfile = fopen(fileString,"w");

    if(newfile == NULL){	//Check, if we cant create the file we have an error and exit
            printf("Unable to create file\n");
			exit(0);
    }

	//If we are given a port we set the port, otherwise we set it equal to the default of 7
	if(argc == 4)
		servPort = atoi(argv[3]);
	else
		servPort = 7;

	/*Create a socket using TCP*/
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		DieWithError("socket() failed");

	/*Construct the server address structure*/
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(servIP);
	servAddr.sin_port = htons(servPort);

	/*Establish connection to echo server*/
	if (connect(sock, (struct scokaddr*) &servAddr, sizeof (servAddr))<0)
		DieWithError("connect( ) failed");

	printf("connect succeed\n");
	//We save the length of our filename
	fileStringLen = strlen(fileString);

	/*send the filename*/
	if (send(sock, fileString, fileStringLen, 0)!=fileStringLen){
		DieWithError("send() sent a different number of bytes than expected");
    }

	printf("Original Send Succeed\n");
	/*Receive the same string back from the server*/

	int stop = 0;	//Stop is used to singal when our client should stop recieving messages
	int checksum = -1;	//Checksum for later
	int serverCheckSum = -1;
	
	while (stop == 0)
	{
		if((bytesRcvd = recv(sock, buff, RCVBUFSIZE, 0))<=0)	//We recieve a segment 
			DieWithError("recv() failed or connecxstion closed prematurely");
		

		if(strncmp(buff,"Requested file does not exist\n\0",bytesRcvd) == 0){ 	//If our segment is the error message saying our file does not exist we set stop = to 1 so we know we have ot leave and exit
			stop = 1;
		}
		else{// We have recieved a valid message that is part of the file
			//printf("bytesRcvd=%d\n", bytesRcvd); //For testing
			//printf("Received:");
			//printf(buff);
			for(int j = 0;j<bytesRcvd;j++){	//We do a type of checksum on our file by adding all the ascii values and moding them by the same number the sever does
				checksum = (checksum + (int)buff[j])%346742;
			}
			//printf("\nStop Before: %d\n",stop);
			if((recv(sock, &stop, sizeof(stop), 0))<=0)	//The server also sends the client a stop message, this will be 0 if there is still more to send, or 1 if it is done
				DieWithError("recv() failed or connection closed prematurely");

			fprintf(newfile,"%s",buff);	//Actually writes our file to the buffer
		}
		
	}
	if(checksum > -1){
		recv(sock, &serverCheckSum, sizeof(serverCheckSum), 0);
		printf("\nServer Checksum: %d",serverCheckSum);
		//sleep(1);
		if(serverCheckSum = checksum){
			printf("\nClient has sucessfully recieved Message");
		}
		else{
			printf("\nClient was not sucessfull in recieving Message");
		}
	}
    if(strcmp(buff,"Requested file does not exist\n\0") == 0){	//If our buff was equal to this
            if(newfile == NULL){	//The file was never created so we do nothing (Safey check)
                printf("File was never created\n");
            }
            else{		//we remove the file 
                printf("Removing File\n");
                remove(fileString);
            }
        
    }

	printf("\n");

	close(sock);	//Close the socket
	exit(0);
} 
