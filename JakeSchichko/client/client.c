#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define BUFFMAX 255 /*Longest string to echo*/
void DieWithError(char *errorMessage);
int main(int argc, char *argv[])
{
    int sock; /*Socket descriptor*/
    struct sockaddr_in ServAddr; /* Echo server address */
    struct sockaddr_in fromAddr; /* Source address of client */
    unsigned short ServPort;    //The Port of the Server
    unsigned int fromSize;  //Size 
    char *servIP;   //The IP of the server
    char *fileName; //The file name we are looking for
    char packetBuffer[BUFFMAX+1];   //The Buffer we are going to be using
    unsigned int fileStringLen; //The length of the filename
    int resStringLen;   //The length of the response message from the server

    int stop = 0;   //Used to stop reciving messages after we have reached the end of the file

    int packetNum= 0;   //Used for sending ack numbers back

    int lastRecievedPacket = -1;    //The number of the last packet recieved, this starts as -1 because we have not recieved anything

    int checksum = 0;   //The checksum we are calculating here in the client, will never add one to it like is done in the server
    int checkSumFromServer = 0; //The checksum that is send over from the server

    double probabiltyQ = 0;    //The probabilty of dropping a packet

    srand(time(0));    //Sets the random function

    if (argc != 5)  //Checks for the correct amount of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <File Name> [<Server Port>] <Probabilty q>\n"), argv[0];
        exit(1);
    }

    servIP = argv[1];   //Assigns the server IP
    fileName = argv[2]; //Assigns the filename
    
    if ((fileStringLen = strlen(fileName)) > BUFFMAX) /* Check input length */
    {
        DieWithError("File name too long"); //Makes sure our filename isnt out of bounds
    }

    if(argc == 5)   //Saftey Check
    {
        ServPort = atoi(argv[3]);   //Saves the port of the server
        probabiltyQ = atof(argv[4]);    //Saves the probability Q
    }
    else    //If some error has occured then we make our default port 7
    {
        ServPort = 7;
    }
    /* Create a datagram/UDP socket */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
    {
        DieWithError("socket() failed");
    }

    /*Construct the server address structure*/
    memset(&ServAddr, 0, sizeof(ServAddr)); /* Zero out structure */
    ServAddr.sin_family = AF_INET;   /* Internet addr family */
    ServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    ServAddr.sin_port = htons(ServPort);  /* Server port */

    FILE *newfile;	//We create a file with the name of the filestring we were given, if this the file does not exist it is later deleated 
    newfile = fopen(fileName,"w");

    if(newfile == NULL){	//Check, if we cant create the file we have an error and exit
            printf("Unable to create file\n");
			exit(0);
    }

    /*send the file name to the server*/
    if (sendto(sock, fileName, fileStringLen, 0, (struct sockaddr *)&ServAddr, sizeof(ServAddr))!=fileStringLen)
    {
        DieWithError("send() sent a different number of bytes than expected");
    }


    /*Receive a response*/
    fromSize = sizeof(fromAddr);

    //This goto will be used later if we are supposed to exit
    not_recv: 
    while(stop == 0){
        
        double randomNum =  (double)rand() / (double)RAND_MAX; //Get a random number between 0 and 1
         //printf ("\nNew Loop\n");
        checksum = 0;   //0 out the checksum 
        //Recieve the packet of text (Parts of the File) from the server
        if ((resStringLen = recvfrom(sock, packetBuffer, BUFFMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0)
        {
            DieWithError("recvfrom() failed") ;
        }
        //printf("%d :",resStringLen);

        //This will be sent if the file does not exist on the sever end (Now we must remove the file we created before)
        if(strcmp(packetBuffer,"Requested file does not exist\n\0") == 0){	//If our buff was equal to this
            if(newfile == NULL){	//The file was never created so we do nothing (Safey check)
                printf("File was never created\n");
            }
            else{		//we remove the file 
                printf("Removing File\n");
                remove(fileName);
            }
            stop = 1;   //Signals we should stop
            goto not_recv;    //Go back outside of the while loop so we can end
        }
        //If the file does exist we continue and ignore the above message on every other loop
        
        //Get the checksum of the buffer that was just sent to us, same way that is done in the server file except we dont add 1
        for(int j = 0;j<resStringLen;j++){
				checksum = (checksum + (int)packetBuffer[j])%346742;
		}

        //printf("Checksum:%d\n",checksum);
        
        //We recv the checksum that was sent from the server and save it into a seperate variable
        if ((recvfrom(sock, &checkSumFromServer, __SIZEOF_INT__, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0)
        {
            DieWithError("recvfrom() failed") ;
        }

         //printf("Checksum From Server:%d\n",checkSumFromServer);

        //We recieve the packetNumber, this is going to be used to check for ordering.
        if ((recvfrom(sock, &packetNum, __SIZEOF_INT__, 0, (struct sockaddr *) &fromAddr, &fromSize)) < 0)
        {
            DieWithError("recvfrom() failed") ;
        }
        if (ServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
        {
            fprintf(stderr,"Error: received a packet from unknown source.\n");
            exit(1);
        }
       
        //printf("\nPacket Number:%d\n",packetNum);

        //If our packet number that we have just recieved is the next in order and the checksum is okay, we add the buffer to our file and update our last recieved packet
        if(packetNum == lastRecievedPacket+1 && (checksum == checkSumFromServer)){
            //printf("\nPacket Correctly Recieved\n");
            lastRecievedPacket = packetNum; //Update our last recieved packet
            fprintf(newfile,"%s",packetBuffer);	//Actually writes our file to the buffer
            //printf("Received:%s\n", packetBuffer); /* Print the echoed arg */
            //printf("Sending ACK: %d\n",lastRecievedPacket);
            if(resStringLen<BUFFMAX){   //If this is less than Buff max we want to stop
                stop = 1;
                //printf("STOPPING");
            }
        }
        else{   //If this is not true, we either recieved an out of order packet or we have a bit error, so we dont update last recieved packet
            //printf("Packet Incorrect\n");
            //printf("Sending ACK: %d\n",lastRecievedPacket);
        }
        
        if(randomNum < probabiltyQ && stop!=1){    //If we have a probabilty that says we have packet loss we want to simulate packet loss but not sending the next ACK packet(This can )
            //printf("Simulating Packet Loss\n");
            // fflush(stdout);
        }

        else{   //Otherwise we send the packet
            //We ack the last correctly recieved Packet
            //printf("Sending ACK %d\n",lastRecievedPacket);
            if(sendto(sock, &lastRecievedPacket, __SIZEOF_INT__, 0, (struct sockaddr *)&ServAddr, sizeof(ServAddr))!=fileStringLen)
            {
                DieWithError("send() sent a different number of bytes than expected");
            }
        }
        //Flush for error messages and for testing
        fflush(stdout);
        /* null-terminate the received data */
        //packetBuffer[resStringLen] = '\0' ;
       
        
    }
    close(sock);    //Close our socket and exit
    exit(0);
}