#include <stdio.h> /*for printf() and fprintf()*/
#include <sys/socket.h> /*for socket(), connect(), send(), and recv()*/
#include <arpa/inet.h> /*for sockaddr_in and inet_addr()*/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#define BUFFMAX 255   /* Longest string to echo */
FILE *fptr;     //File Descriptor


void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{
    int sock; /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen; /* Length of incoming message */
    char echoBuffer[BUFFMAX]; /* Buffer for echo string */
    unsigned short echoServPort; /* Server port */
    int sendMSGSize; /* Size of received message */

    char buff[BUFFMAX];  //The Buffer which will be used to send and receive messages
    int sendbytes = 0;  //The amount of bytes that are sent to the client

    int sizeOfN = 1;    //The window size, will be changed, however as a default I just put one because it cannot be lower than this
    int sentPackets = 0;    //The amount of packets currently sent
    float probability = 0;  //Probabilty of a Packet having a checksum error

    int ACK = 0;    //The current ACK
    int nextACK = 0;    //What should be the next ACK

    int checksum = 0;   //The Servers Checksum
    
    struct timeval tv;  //Used for setting up timeout;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    srand(time(0));    //Sets the random function

    if (argc != 4) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: <Server Port> <Size of N> <Probability P>\n", argv[0]) ;
        exit(1);
    }
    echoServPort = atoi(argv[1]); /* First arg: local port */
    sizeOfN = atoi(argv[2]);    //Sets the window size
    probability = atof(argv[3]);    //Sets the Probabilty 

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
        //Echo Buffer is the filename

        tv.tv_sec = 0;//Sets the timer timeout to be 0, so we can wait for a filename
        tv.tv_usec = 0;
        if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0){   //Sets the options of the socet
            DieWithError("ERROR");
        }
        if ((sendMSGSize = recvfrom(sock, echoBuffer, BUFFMAX, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed") ;
        }
        //Now we have the filename
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        clock_t t; //Starts our timer
        t = clock();    //Ends our Timer
    
        echoBuffer[sendMSGSize] = '\0'; //Adds the null terminator so we can correctly get the file name
        checksum = 0;   //Resets checksum variable
        sentPackets = 0;    //Resets sentPackets variable
        //Start looking for file

        tv.tv_sec = 1; //Since we are entering the main portion of our program we want to set the timeout to be 1 second, im sure this can be much smaller but having it be 1 second is easy to notice and track packet loss
        //tv.tv_usec = 100000;
        if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0){
            DieWithError("ERROR");
        }

        if(access(echoBuffer, F_OK ) != -1 ) {	//If we can acess the file that the client requested we move in and start reading it into the buffer in segments
             printf("File Exists \n");  //Lets the User know the file exists
             // Open file 
             fptr = fopen(echoBuffer, "r"); //Opens the file to the bugger
              if (fptr != NULL) {
                  //sleep(1);
				 fseek(fptr, 0L, SEEK_END);	//This will let us see how big the file is
				 int sz = ftell(fptr);	//sz becomes the full size of the file
				 int stop = 0;	//If stop = 0 that means we have more to read, if it =1 we are done with our file
				 //printf ("\nTotal Size:%d\n",sz);	//Prints the total file size
				 rewind(fptr);	//Rewinds the file so we can start from the beginning 
                
                 double packetDouble = ceil(((double)sz /((double)BUFFMAX - 1)));   //The amount of packets, needs to be double because we want to round up the decimal, we can not chop it off 
                 int packets = (int)(packetDouble); //change it to a int for simplicity(We cannot have a fraction of a packet anyway)
                 if(packets == 0){  //Saftey Check, we cannot have 0 packets if the file exists there must be one packet
                     packets = 1;
                 }
                 int packetsLeft = packets; //To start we have packets, packetsleft
                 //printf("Number of Packets: %d\n",packets); //Prints total number of packets
                 
                 int bigWindow[packets];    //BigWindow shows what each poisitons packet number should be
                 int bigWindowPositions[packets];   //Big Window position looks at size and stores what the corresponding location in the text each window should look at, for example [0] would look at size, this is usefull when we have loss and need to go back and find a new point
                 for (int i = 0;i<packets;i++){
                     bigWindow[i] = i;  //Packet number
                     bigWindowPositions[i] = sz - (i*(BUFFMAX-1));  //The size 
                     //printf("\nbigWindow %d Position %d",bigWindow[i],bigWindowPositions[i] );
                 }
                
                 
                 int counter[sizeOfN];  //Used to keep track of what packet each point in the window is sending
                 int timers[sizeOfN];

                 sentPackets = 0;       //Resets the amount of sent Packets to 0
                 int freeSpots = sizeOfN;      //Once we send a packet, we must wait for its ack, therefor we have the window size in "Free Space"
                 nextACK = 0;        //Reset nextACK

                 //Counter is used for testing
                 counter[0] = 0;    //Reset to this for testing purposes
                 counter[1] = 1;

                 int sent = 0;  //Used for tracking

                 while(stop == 0){
                    // count = 0;
                    //  while(count == 0){
                    //      int x = counter[0];
                    //      int y = counter[1];
                    //       printf("\nARRAY:%d %d\n",bigWindow[x],bigWindow[y]);
                    //        printf("Sizes:%d %d\n",bigWindowPositions[x],bigWindowPositions[y]);

                    //     scanf("%d",&count);
                    //  }
                    

                     //printf ("\nNew Loop Packets Left: %d\n",packetsLeft);
                     checksum = 0;  //Resets Checksum each loop because we have a new packet
                    

                    if(freeSpots > 0 && sz>0){  //If we have a spot available and are not at the end of our file we must sent a packet
                        double random =  (double)rand() / (double)RAND_MAX; //Get a random number between 0 and 1
                        //printf("Random %f\n",random);
                        
                        buff[0] = '\0'; //Reset the Buffer
                        //printf("\nSending Packet Number: %d\n",sentPackets);
                        
                        //Gets the size that should be sent, this should be equal to BUFFMAX -1 unless its the end of the file
                        sendMSGSize = fread(buff, sizeof(char), BUFFMAX - 1, fptr);	//We read the buffer and add however much of it we can fit knowing the buffer size, for example if it is more than 22556 we can only fit 255 bytes in the segment to account for the null terminator
                 
                            //A saftey check for reading the file
                            if ( ferror( fptr ) != 0 ) {
                                fputs("Error reading file", stderr);
                            } else {
                                buff[sendMSGSize++] = '\0'; /* Just to be safe. */
                            }
                            //printf("sendMSgSize:%d\n",sendMSGSize);
                            //printf("\nBuff:%s\n",buff);
                            //fflush for testing
                            fflush(stdout);

                            //Here we have my madeup checksum function, just doing some random functiona and getting a value
                            for(int j = 0;j<sendMSGSize;j++){
                                checksum = (checksum + (int)buff[j])%346742;
                            }
                            //If we have a bit error, we add one to the checksum to simulate a bit error(as the client will not do this)
                            if(random <probability){
                                //printf("Bitflip!\n");
                                checksum = checksum +1;
                            }
                            //printf ("Checksum: %d\n",checksum);

                            //We send our buffer to the client, if the sendbyte is different than the calculated message we have an error
                            sendbytes = sendto(sock, buff, sendMSGSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));	//Sends the client the current buffer
                            if (sendbytes != sendMSGSize){	//A check to see if something went wrong with the send 
                                DieWithError("send() failed");
                            }
                             sz = sz - (sendMSGSize - 1); //We decrease the size by the amount we have just sent

                            sendto(sock, &checksum, __SIZEOF_INT__, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));	//Sends the client the checksum value
                        
                            fflush(stdout);//For testing

                            //Send the Client What packet number they have just recieved
                            sendto(sock, &sentPackets, __SIZEOF_INT__, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));
                            

                            //printf("\n");
                            sent ++;        //For test
                            freeSpots --;   //We have send something so we decrease the amount of freespots we have
                            sentPackets ++; //And increase the amount of packets we have sent
                    
                    }


                    //When we have nothing more to send we just wait and listen for acks
                    else{
                        //printf("HERE");
                        //Recieve the ACK
                        fflush(stdout);
                       if ((recvfrom(sock, &ACK, __SIZEOF_INT__, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
                       {
                           //printf("Packet Loss, going back\n");  //This happens to deal with packet loss
                           //printf("nextACK: %d\n",nextACK);
                            fflush(stdout);
                           sz = bigWindowPositions[nextACK];
                           sentPackets = nextACK;       //We reset the next ACK to what should be the next packet that was lost
                           freeSpots = sizeOfN -1;  //We open up all the spots again and send over everything (-1 because we add another 1 later in this else statement)
                           fseek(fptr, -1*(sz), SEEK_END);
                       };

                        //printf("ACK:%d\n",ACK);
                       //printf("NEXT ACK:%d",nextACK);

                       //If the ACK is the wrong ACK, we resend our last message
                       if(ACK != nextACK & ACK < nextACK){
                           //printf("IN CHECKSUM ERROR");
                            //printf("ACK %d\n",ACK);
                           //printf("Must wait\n");
                           //printf("ACK:%d",ACK);
                           //printf("Size of N: %d",sizeOfN);
                           //printf("Must go to byte %d Bytes",bigWindowPositions[nextACK]);
                           
                           sz = bigWindowPositions[nextACK];    //We go back to the last messages position in the file
                           fseek(fptr, -1*(sz), SEEK_END);      //And move our pointer to that size
                           sentPackets = nextACK;   //We also move back to only having nextACK packets send, we must retransmit all packets that happened before this wrong ack
                       }
                       else{
                          //printf("ACK:%d\n",ACK);
                          packetsLeft --;   //Else we sucessfully recieved a packet and can decrease the amount left
                          
                          nextACK ++;   //Move the next ACK up by one
                          sent = sent %sizeOfN; //For testing
                          //printf("Counter Send BEFORE:%d\n",counter[sent]);
                          counter[sent] = nextACK + 1;  //Used for tracking which packet number belongs where
                          //printf("Counter Send AFTER:%d\n",counter[sent]);
                       }
                       freeSpots ++;    //Either way, we have another free spot and can send another packet, whether it is a retransmission or a new packet

                       //If our size is small and we have no packetsLeft, we send
                       if(sz <= 4 && packetsLeft ==0 || nextACK>(packets-1)){
                           stop = 1;    //Stops our loop
                           //printf("STOPPING\n");
                           fflush(stdout);
                       }
                    }
				
				 }
                
                    t = clock() - t; //Used for timing
                    double time_taken = ((double)t)/CLOCKS_PER_SEC;
                    //printf("Time Taken:%lf\n",time_taken); //COMMENT THIS IN TO SEE TIME
                  fclose(fptr);	//Close the file
              }

        }
        else{
             printf("Requested file does not Exist \n"); //This happens if the file does not exist
             //n = 0; 
             // copy server message in the buffer 
             sprintf(buff, "Requested file does not exist\n\0"); 
			 sendMSGSize = strlen(buff);
             printf("CHECK:%d\n",sendMSGSize);
             // and send that buffer to client 
             sendbytes = sendto(sock, buff, sendMSGSize, 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr));	//Sends the client the current buffer		
        }
    }
    /* NOT REACHED */
}
