//Import Necessary Packages
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

//Define Method Headers
int goBack(int cLength, struct sockaddr_in client, int sd, char *buf, int numPackets, int dataSize, int bytes, int dropRate);
int getBack(int *cLength, struct sockaddr_in *client, int sd, char *buf, int *dataSize, int dropRate);
int stopWaitSend(int cLength, struct sockaddr_in client, int sd, char *buf, int numPackets, int dataSize, int bytes, int dropRate);
int stopWaitReceive(int *cLength, struct sockaddr_in *client, int sd, char *buf, int *dataSize, int dropRate);

//Helper Method To Calculate Number Of Packets Left
int calculateNumFrames(int bytes, int dataSize){
        int numPackets;
        numPackets = bytes/dataSize;

        //If We Need Another Packet
        if(numPackets*dataSize < bytes){
		//Increment Number Of Packets
                numPackets++;
        }
   return numPackets;
}

//Helper Method To Read In A File
int readFile(char *servBuffer, char *filename){
	//Declare Necessary Variables
        int bytes = 0;
        int totBytes = 0;
        int fd = 0;

        //Open The File
        fd = open(filename, O_RDONLY);
	//Check If File Opened Correctly
        if (fd < 0) {
                printf("Not Able To Open File\n");
                return 0;
        }

	//While Loop To Read The File
        while(1 == 1){
		//Read From File
                bytes = read(fd, servBuffer, 1048576);
                //Check For The End
		if (bytes <= 0){
                        break;
                }
		//Increment Byte Counter
                totBytes += bytes;
        }
	//Close File
        close(fd);
	//Return Number Of Bytes In File
        return totBytes;
}

//Main Method
int main(int argc, char **argv){

	//Declare Necessary Variables
	struct sockaddr_in server;
	struct sockaddr_in client;
   	int sock = 0;
	int sockCheck = 0;
	int cLength = 0;
	int n = 0;
	int port = 5000;
	int dropRate = 0;
	int protocol = 0;
	int numPackets = 0;
	int dataSize = 0;
	int bytes = 0;
	int fnameBytes = 0;
        int errno = 0;
   	char recBuffer[1000000];
	char buf[1000000];
	char *scriptName;
        char *filename;
        char dataSize_str[5];

	//Initialize Random Number Generator
	srand(time(NULL));

	//Strip Script Name From Argument List
	scriptName = argv[0];
	argc--;

	//Check Number Of Arguments
	if (argc > 0){
		//Get The Port Number To Use
		if(strcmp(*++argv, "-p") == 0) {
			port = atoi(*++argv);
			argc -= 2;
       		}else{
         		--argv;
       		}
       		//Get The Drop Rate And Protocol
		if(argc == 2){
        		dropRate = atoi(*++argv);
        		protocol = atoi(*++argv);
       		}else{
			//If Incorrect Number Of Arguments, Print Usage Message
        		printf("Inccorect Number Of Arguments.\n");
			printf("To Run Server Side Use The Following Command: \n");
                	printf("Usage: ./script-Name -p port-number drop-rate protocol\n");
                	printf("EXAMPLE: ./udp_server.o -p 80 25 (1 or 2)\n");
        		exit(1);
       		}
    	}else{
		//If Incorrect Number Of Arguments, Print Usage Message
       		printf("Incorrect Number Of Arguments.\n");
		printf("To Run Server Side Use The Following Command: \n");
		printf("Usage: ./script-Name -p port-number drop-rate protocol\n");
		printf("EXAMPLE: ./udp_server.o -p 80 25 (1 or 2)\n");
       		exit(1);
    	}

   	while(1 == 1){
     		//Create A Socket
		sock = socket(AF_INET, SOCK_DGRAM, 0);
     		//Test If Socket Created, If Not Exit
		if (sock == -1){
        		printf("Socket Creation Failed.\n");
        		exit(1);
     		}


     		//Bind To Socket
     		bzero((char *)&server, sizeof(server));
     		server.sin_family = AF_INET;
     		server.sin_port = htons(port);
     		server.sin_addr.s_addr = htonl(INADDR_ANY);
		
		//If We Can't Bind To Socket, Exit
		sockCheck = bind(sock, (struct sockaddr *)&server, sizeof(server));
     		if(sockCheck == -1){
        		printf("Socket Binding Failed.\n");
        		exit(1);
     		}
		//Get Client Length
      		cLength = sizeof(client);

		if(protocol == 1){

			printf("\n\n---------------------------------------------START SERVER SIDE: STOP AND WAIT---------------------------------------------------------\n\n");
          		printf("Getting Filename From Client...\n");
          		
			//Call Stop And Wait To Get Filename
			fnameBytes = stopWaitReceive(&cLength, &client, sock, recBuffer, &dataSize, dropRate);
          		if(fnameBytes == -1){
            			printf("Not Able To Get Filename From Client..\n");
				printf("Please Try Again...\n");
            			exit(1);
          		}
			
			//Use Memcopy To Get Filename
          		memcpy(dataSize_str, recBuffer, 5);
          		printf("Done. Filename Obtained...\n\n");
			
			//Read Bytes From Obtained File, Set Data Size, And Calculate The Number Of Packets Needed
          		bytes = readFile(buf, (recBuffer+6) );
          		dataSize = atoi(dataSize_str);
          		numPackets = calculateNumFrames(bytes, dataSize);
          		
			//Call The Stop And Wait Protocol
          		stopWaitSend(cLength, client, sock, buf, numPackets, dataSize, bytes, dropRate);
			printf("\n\n-----------------------------------------------END SERVER SIDE: STOP AND WAIT---------------------------------------------------------\n\n");
			fflush(stdout);	
		}else if(protocol == 2){

			printf("\n\n---------------------------------------------START SERVER SIDE: GO BACK N---------------------------------------------------------\n\n");
          		printf("Getting Filename From Client...\n");
          		
			//Call Go Back N To Get Filename
			fnameBytes = getBack(&cLength, &client, sock, recBuffer, &dataSize, dropRate);
          		if(fnameBytes == -1) {
            			printf("Not Able To Get Filename From Client...\n");
				printf("Please Try Again...\n");
				exit(1);
          		}
          		
			//Use Memcopy To Get Filename
          		memcpy(dataSize_str, recBuffer, 5);
          		printf("Done. FIlename Obtained...\n\n");
			
			//Read Data From Obtained File, Set Data Size, And Calculate Number Of Packets Needed
          		bytes = readFile(buf, (recBuffer+6) );
          		dataSize = atoi(dataSize_str);
          		numPackets = calculateNumFrames(bytes, dataSize);
          		//Call The Go Back N Protocol
          		goBack(cLength, client, sock, buf, numPackets, dataSize, bytes, dropRate);
			printf("\n\n---------------------------------------------END SERVER SIDE: GO BACK N---------------------------------------------------------\n\n");
			fflush(stdout);
		}else{
		
			//If Protocol Isn't Stop And Wait Or Go Back N, Print Usage Message
			printf("That Is Not A Recognized Protocol.\n");
			printf("To Run Server Side Use The Following Command: \n");
                        printf("Usage: ./Script-Name -p port-number drop-rate protocol\n");
                        printf("EXAMPLE: ./server.o -p 80 25 (1 or 2)\n");
                        exit(1);
		}
	//Close File
	close(sock);
	}//End While
   	return(0);
}//End Main
