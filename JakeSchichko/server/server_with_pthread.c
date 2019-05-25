#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> 
#define BUFFMAX 255   /* Longest string to echo */
FILE *fptr; 


typedef struct {
    //Or whatever information that you need
    int sock;
    struct sockaddr_in echoClntAddr;
    unsigned int cliAddrLen;
}   my_struct;


void DieWithError(char *errorMessage); /* Error handling function */
void threadSend(void *args);
int main(int argc, char *argv[])
{
    int sock; /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[BUFFMAX]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int sendMSGSize; /* Size of received message */

    char buff[BUFFMAX];  
    int sendbytes = 0;

    int sizeOfN = 1;
    int sentPackets = 0;
    float probability = 0;

    int ACK = 0;
    int lastACK = 0;
    if (argc != 4) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server Port> <Size of N> <Probability P>\n", argv[0]) ;
        exit(1);
    }
    echoServPort = atoi(argv[1]); /* First arg: local port */
    sizeOfN = atoi(argv[2]);
    probability = atof(argv[3]);

    printf("Size of N %d",sizeOfN);
    printf("Probability %f",probability);
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        DieWithError( "socket () failed") ;
    }
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out 
    structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming 
    interface */
    echoServAddr.sin_port = htons(echoServPort); /* Local port */
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0){
        DieWithError ( "bind () failed");
    }

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        /* Block until receive message from a client */
        if ((sendMSGSize = recvfrom(sock, echoBuffer, BUFFMAX, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed") ;
        }
        //Now we have the filename
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        echoBuffer[sendMSGSize] = '\0'; //Adds the null terminator so we can correctly get the file name

        //Start looking for file
        if(access(echoBuffer, F_OK ) != -1 ) {	//If we can acess the file that the client requested we move in and start reading it into the buffer in segments
             printf("File Exists \n"); 
             // Open file 
             fptr = fopen(echoBuffer, "r"); //Opens the file to the bugger
              if (fptr != NULL) {
				 fseek(fptr, 0L, SEEK_END);	//This will let us see how big the file is
				 int sz = ftell(fptr);	//sz becomes the full size of the file
				 int outbytes = 0; 	// used in testing
				 int stop = 0;	//If stop = 0 that means we have more to read, if it =1 we are done with our file
				 printf ("\nTotal Size:%d\n",sz);	//Prints the total file size
				 rewind(fptr);	//Rewinds the file so we can start from the beginning 
                 int packets = sz / (BUFFMAX - 1);

                 printf("Number of Packets: %d",packets);
                  my_struct *args = malloc(sizeof *args);
                  args->sock = sock;
                  args->echoClntAddr = echoClntAddr;
                  args->cliAddrLen = cliAddrLen;
                 pthread_t thread_id[sizeOfN]; 
                 printf("Before Thread\n"); 
                 for(int i = 0;i<sizeOfN;i++){
                    pthread_create(&thread_id[i], NULL, threadSend, args); 
                 }
                 for(int i = 0;i<sizeOfN;i++){
                    pthread_join(thread_id[i], NULL); 
                 }
                 printf("After Thread\n"); 
                
                 fclose(fptr);	//Close the file
              }

        }
        else{
             printf("Requested file does not Exist \n"); 
             //n = 0; 
             // copy server message in the buffer 
             sprintf(buff, "Requested file does not exist\n\0"); 
			 sendMSGSize = strlen(buff);
             printf("CHECK:%d\n",sendMSGSize);
             // and send that buffer to client 
             sendbytes = sendto(sock, buff, sendMSGSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));	//Sends the client the current buffer		
        }

        /* Send received datagram back to the client */
        if (sendto(sock, echoBuffer, sendMSGSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sendMSGSize){
            DieWithError("sendto() sent a different number of bytes than expected");
        }
    }
    /* NOT REACHED */
}


void threadSend(void *args){
    my_struct *myArgs = args;
    int sock = myArgs->sock; /* Socket */
    unsigned int cliAddrLen = myArgs->cliAddrLen; /* Local address */
    struct sockaddr_in echoClntAddr = myArgs->echoClntAddr; /* Client address */
    char buff[BUFFMAX];  
    int sendMSGSize = 0;
    int stop = 0;
    int sendbytes = 0;
    //while(sz > 0 && stop == 0){
					//send(clntSocket,&stop,sizeof(int),0);
                    //Sends for each window size
                    //sendCounter = 0;
                   // while((sendCounter<sizeOfN) && sz>0){
                        buff[0] = '\0';
                        printf("\nSending Packet Number:\n");
                        sendMSGSize = fread(buff, sizeof(char), BUFFMAX - 1, fptr);	//We read the buffer and add however much of it we can fit knowing the buffer size, for example if it is more than 256 we can only fit 255 bytes in the segment to account for the null terminator
                        if(sendMSGSize < 254){
                            stop =1;
                        }
                    
                        
                        //A saftey check for reading the file
                        if ( ferror( fptr ) != 0 ) {
                            fputs("Error reading file", stderr);
                        } else {
                            buff[sendMSGSize++] = '\0'; /* Just to be safe. */
                        }
                        printf("sendMSgSize:%d\n",sendMSGSize);
                        printf("Buff:%s\n",buff);
                        fflush(stdout);

                        sendbytes = sendto(sock, buff, sendMSGSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));	//Sends the client the current buffer
                        if (sendbytes != sendMSGSize){	//A check to see if something went wrong with the send 
                            DieWithError("send() failed");
                        }

                        //sz = sz - (sendMSGSize - 1); //We decrease the size by the amount we have just sent
                        fflush(stdout);
                        sendto(sock, &stop, __SIZEOF_INT__, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));
                        //sendCounter ++;
                       // sentPackets ++;
                   // }

                    // for(int i = 0; i<sentPackets;i++){
                    //    if ((recvfrom(sock, &ACK, __SIZEOF_INT__, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
                    //    {
                    //        DieWithError("recvfrom() failed") ;
                    //    };
                    //    printf("ACK:%d\n",ACK);
                    //    if(lastACK != ACK +1 && ACK !=0){
                    //        printf("Must wait\n");
                    //        printf("Must go back %d Bytes",(sizeOfN-ACK)*254);
                    //    }
                    //    else{
                    //       printf("ACK:%d\n",ACK);
                    //       lastACK = ACK;
                    //    }
                    // }
				
				// }

}