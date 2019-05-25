List of relevant file: 
client
client.c
client.o
server
server.c
server.o
DieWithError.c
DieWithError.o
HandleTCPClient.c 
HandleTCPClient.o
makefile

(Optinal testing text files)


Compilation instuctions:
I have split the client and server into two folders, if the client attemps to write to a file the server is reading from it will cause an error

(In Client Folder)
make client
(In Server Folder)
make server
(In Server Folder)
./server 80
(In Client Folder)
./client 127.0.0.1 test.txt 80

Configuration files: None used


Running Instructions:
(In Server Folder)
./server 80
#Note: This executable starts running the server on port 80, this can be changed to whatever port # is desired
(In Client Folder)
./client 127.0.0.1 test.txt 80
#Note: This executable starts running the client looking for IP 127.0.0.1 port 80, this can be changed to whatever port # and IP is desired. The argument in the middle of the port and the IP is the file that the client is looking for, this can also be changed, if it does not exist the client will be notified of this

