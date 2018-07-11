#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

int main(int argc, char **argv){
	//Declare Necessary Variables
	struct hostent *host;
	struct sockaddr_in channel;
	int chan = 0;
	int sock = 0;
       	int bytes = 0;
 	char buffer[4096];
  	char outBuffer[4096];   
	FILE *destFile;

	//Check Number Of Arguments
  	if(argc != 3){
		printf("Usage: ./client.o [SERVER] [FILE]");
		exit(1);
	}
  	
	//Get host IP From Arguments
	host = gethostbyname(argv[1]);
  	
	//Check If Host Was Able To Connect
	if(!host){
	       	printf("Crash..\n");
		printf("There Was A Problem Getting The Host's IP..\n");
	}

	//Set Up Socket and Channel, Try To Connect
  	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  	memset(&channel, 0, sizeof(channel));
  	channel.sin_family= AF_INET;
  	memcpy(&channel.sin_addr.s_addr, host->h_addr, host->h_length);
  	channel.sin_port= htons(2910);
  	chan = connect(sock, (struct sockaddr *) &channel, sizeof(channel));
  	
	//If Socket Or Channel Are Unable To Connect, Print Message
	if((sock < 0) || (chan < 0)){
		printf("Crash..\n");
       		printf("There Was A Problem With Setting Up The Socket..\n");
 		printf("Or A Problem Connecting..\n");		
	}//End If

 	//We Have A Connection, So Write Input File To Local File Destination
  	write(sock, argv[2], strlen(argv[2])+1);
	sprintf(outBuffer, "results/new_%s", argv[2]);
	destFile = fopen(outBuffer, "w+");
  
	//While Loop To Get Data
	while(1 == 1){
		//Begin Reading From Socket
        	bytes = read(sock, buffer, 4096);
		//If We Have Nothing Left In File, Exit
        	if(bytes <= 0){
		       exit(0);
		}
		//Write Buffer To Destination
        	fwrite(buffer, sizeof(char), bytes, destFile);
  	}//End While
	printf("Client Is Finished Receiving..\n");
	fflush(stdout);
	//Close Destination File
  	fclose(destFile);

}//End Main
