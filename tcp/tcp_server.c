#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


//Main Method For TCP Server
int main(int argc, char *argv[]){	

	//Declare Necessary Variables
	struct sockaddr_in channel;
	int sock = 0;
	int bindSock = 0;
	int listSock = 0;
	int dataFile = 0;
       	int accSock = 0;
	int bytes = 0;
	int on = 1;
	char buffer[4096];	

	//Build Address Structure To Creat Socket
	memset(&channel, 0, sizeof(channel));
	channel.sin_family = AF_INET;
	channel.sin_addr.s_addr = htonl(INADDR_ANY);
	channel.sin_port = htons(2910);

	//Create Socket, Bind To Socket, And Listen On Socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 	bindSock = bind(sock, (struct sockaddr *) &channel, sizeof(channel)); 
	listSock = listen(sock, 10);

	//If Any Of The Above Connections Fail, Then Print Error
	if (((socket < 0) || (bindSock < 0) || (listSock < 0))){
		printf("Crash...\n");
		printf("Problem Creating Socket..\n");
		printf("Or Binding To Socket..\n");
		printf("Or Listening..\n");
	}
  	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

  	//While Loop To Accept Connection From Client
  	while (1 == 1) {

		//Block For Connection
        	accSock = accept(sock, 0, 0);
		
		//Read File From Socket
        	read(accSock, buffer, 4096);

        	//Open File To Get Data
        	dataFile = open(buffer, O_RDONLY);

        	while(1 == 1){

			//Read File
                	bytes = read(dataFile, buffer, 4096);

			//Check For End Of File
                	if (bytes <= 0){
			       	break;
	       		}

			//Write Bytes To Destination
                	write(accSock, buffer, bytes);

        	}//End While

		printf("Finished...\n");
		printf("Ready For Next File..\n");
		fflush(stdout);
		
		//Close File And Connection
        	close(dataFile);
        	close(accSock);

  	}//End While
}//End Main
