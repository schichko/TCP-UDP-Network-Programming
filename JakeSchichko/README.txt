List of Relavant Files:
client
client.c
server
server.c
DieWithError.c
Makefile

(Optional testing Text files, I have provided one called file)

Compilation Instructions:
I have split the client and server into two folders because, if the client attempts to write to a file the server is reading from it will cause an error

(In Client Folder)
make client
(In Server Folder)
make server
(In Server Folder)
./server 80 2 .3 //Where 2 is the Window size and .3 is the probabilty of a checksum error
(In the Client Folder)
./client 127.0.0.1 file 80 .2 //Where .2 is the q value, or the probabilty of a packet loss NOTE: THIS SHOULD BE 0 IF NOT CHECKING THE  BONUS 

Configuration Files used:
I did not use any configuration files however in Makefile i do        -pthread and -lm (Although i did not end up using pthread)

Running Instructions
(In Server Folder)
./server 80 2 .3
#Note: this executable starts running the server on port 80, with a window size of 2 and a 30% change of a checksum error, this can be changed to whatever values are desired

(In Client Folder)
./client 127.0.0.1 file 80 .2
#Note: this executable starts running the client with a connection to 127.0.0.1 on port 80. Its looking for the file "file" and has a 20% chance of dropping a packet. These values can be changed to whatever is desired


