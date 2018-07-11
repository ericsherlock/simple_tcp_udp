//Import Necessary Packages
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

//Helper Method To Write To Destination File
int writeFile(char *servBuffer, char *filename, int filesize){
	//Declare Necessary Variables
        int bytes = 0;
        FILE *dst;
        //Open File For Writing, Write To Destination, Close FIle
        dst = fopen(filename, "w+");
        bytes = fwrite(servBuffer, filesize*sizeof(char), 1, dst);
        fclose(dst);

        return bytes;
}

int main(int argc, char **argv){
	//Declare Necessary Variables
	struct hostent *hostName;
	struct sockaddr_in server;
	struct timeval start;
	struct timeval end;
	int dataSize = 0;
	int port = 5000;
	int protocol = 0;
	int i = 0;
	int sock = 0;
	int servLength = 0;
	int bytes = 0;
        int numPackets = 0;
	int fnameBytes = 0;
	int errno = 0;
	char *scriptName;
	char *host;
        char *srcFile;
	char *dstFile;
       	char recBuffer[1000000];
        char servBuffer[1000000];
	unsigned long address;

	//Initiallize Random Number Generator
	srand(time(NULL));

	//Strip Script Name From Argument List
	scriptName = argv[0];
	argc--;
	argv++;
    
	//Get Data Size Argument
	if(argc > 0 && (strcmp(*argv, "-p") == 0)){
		if(--argc > 0 && (port = atoi(*++argv))){
			argc--;
			argv++;
		}else{
			//Print Usage Message
			printf("Incorrect Number Of Arguments\n");
		        printf("To Run Client Side Use The Following Command: \n");
        		printf("Usage: ./script-name -s data-size host -p port source-filename destination-filename protocol\n");
        		printf("EXAMPLE: ./client -p 80 localhost -s 512 test_file copy_test_file (1 or 2)\n");
			exit(1);
       		}
    	}

	//Get Other Arguments: Filenames, protocol
	if(argc > 0){
		host = *argv;
		if (--argc > 0) {
			//Get The Port Number, Decrement Number Of Arguments
			if(strcmp(*++argv, "-s") == 0) {
				dataSize = atoi(*++argv);
				argc -= 2;
			}else{
				argv--;
        		}
		}else{
                        //Print Usage Message
                        printf("Incorrect Number Of Arguments\n");
                        printf("To Run Client Side Use The Following Command: \n");
                        printf("Usage: ./script-name -p port host -s data-size source-filename destination-filename protocol\n");
                        printf("EXAMPLE: ./client -p 80 localhost -s 512 test_file copy_test_file (1 or 2)\n");
			exit(1);
		}
		//Get Filenames, Protocol
       		if(argc == 3) {
			srcFile = *++argv;
			dstFile = *++argv;
			protocol = atoi(*++argv);
       		}else{
			//Print Usage Message
		        printf("There Was A Problem..\n");
        		printf("To Run Client Side Use The Following Command: \n");
        		printf("Usage: ./script-name -s data-size host -p port source-filename destination-filename protocol\n");
        		printf("EXAMPLE: ./client -p 80 localhost -s 512 test_file copy_test_file (1 or 2)\n");
        		exit(1);
       		}
    	}else{
        	printf("There Was A Problem..\n");
        	printf("To Run Client Side Use The Following Command: \n");
        	printf("Usage: ./script-name -s data-size host -p port source-filename destination-filename protocol\n");
        	printf("EXAMPLE: ./client -p 80 localhost -s 512 test_file copy_test_file (1 or 2)\n");
       		exit(1);
    	}

	//Create A Socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	//Test if Socket Created, If Not Exit
	if(sock == -1){
		printf("Can't create a socket\n");
		exit(1);
    	}

	//Bind To Socket
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Get The Hostname
	hostName = gethostbyname(host);
	//If Hostname Is Empty, Exit
	if (hostName == NULL) {
		printf("Not Able To Get Hostname.\n");
		printf("Please Try Again...\n");
		exit(1);
	}

	//Get Hostname Address
	bcopy(hostName->h_addr, (char *) &server.sin_addr, hostName->h_length);
 
	//Check Packet Size Given By User
	if(dataSize > 4096){
		printf("Data is too big\n");
		exit(1);
    	}

	//Get Size Of Filename, Server, And Number Of Packets Needed
	bytes = sizeof(srcFile)+1;
	servLength = sizeof(server);
	numPackets = calculateNumFrames(bytes, dataSize);
	//Put Bytes Into Buffer
	bytes = sprintf(servBuffer, "%05d,%s", dataSize, srcFile);

	if(protocol == 1){
		
		printf("\n\n---------------------------------------------START CLIENT SIDE: STOP AND WAIT---------------------------------------------------------\n\n");
			
			printf("\nRequesting '%s' From Server...\n\n", srcFile);
			//Get Filename From Server
			fnameBytes = stopWaitSend(servLength, server, sock, servBuffer, numPackets, dataSize, bytes, 0);
			//If Can't Obtain Filename, Print Error, Exit
			if(fnameBytes == -1){
				printf("Not Able To Get Filename From Server...\n");
				printf("Please Try Again...\n");
				exit(1);
        		}
			//Receive File From Server
        		printf("Receiving File...\n");
        		bytes = stopWaitReceive(&servLength, &server, sock, recBuffer, &dataSize, 0);
        		printf("File Received...\n");
			
			//Check The Amount Of Data Is Correct
                        dataSize = writeFile(recBuffer, dstFile, bytes);
                        if(dataSize == 1){
                                printf("%dB Written To: %s\n", bytes, dstFile);
                        }else{
                                printf("Error. Only Able To Write %dB To: %s\n", bytes, dstFile);
                        }
			
			printf("\n\n-----------------------------------------------END CLIENT SIDE: STOP AND WAIT---------------------------------------------------------\n\n");
       
	}else if(protocol == 2){
			
			printf("\n\n---------------------------------------------START SERVER SIDE: GO BACK N---------------------------------------------------------\n\n");
        		
			printf("\nRequesting '%s' From Server...\n\n", srcFile);
			//Get Filename From Server
			fnameBytes = goBack(servLength, server, sock, servBuffer, numPackets, dataSize, bytes, 0);
			//If Can't Obtain Filename, Print Error, Exit
        		if(fnameBytes == -1){
          			printf("Not Able To Get Filename From Server...\n");
				printf("Please Try Again...\n");
          			exit(1);
        		}
			//Receive File From Server
        		printf("Receiving File...\n");
        		bytes = getBack(&servLength, &server, sock, recBuffer, &dataSize, 0);
        		printf("File Received...\n");

			//Check The Amount Of Data 
			dataSize = writeFile(recBuffer, dstFile, bytes);
		        if(dataSize == 1){
                		printf("%dB Written To: %s\n", bytes, dstFile);
        		}else{
                		printf("Error. Only Able To Write %dB To: %s\n", bytes, dstFile);
        		}
			
			printf("\n\n---------------------------------------------END CLIENT SIDE: GO BACK N---------------------------------------------------------\n\n");
    	}
	//Close Socket
	close(sock);
	return(0);
}
