//Declare Necessary Packages
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#define max(a,b) \
	({__typeof__ (a) _a = (a); \
	 __typeof__ (b) _b = (b); \
	 a > _b ? _a : _b; })

//Declare Function Headers
int goBack(int client_len, struct sockaddr_in client, int sd, char *buf, int num_packets, int data_size, int bytes, int dropRate);
int getBack(int *client_len, struct sockaddr_in *client, int sd,char *buf, int *data_size, int dropRate);
int stopWaitSend(int client_len, struct sockaddr_in client, int sd, char *buf, int num_packets, int data_size, int bytes, int dropRate);
int stopWaitReceive(int *client_len, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate);


//Packet Structure
struct packet{
	//Set Packet Number, Length, Number Of Packets, And A Data Array To Hold Data
	uint32_t packNum;
	uint32_t len;
	uint32_t numPackets;
  	char data[4096];
};

//Acknowledge Packet Structure
struct ack{
	//Set Packet Number, Length, And Number Of Bytes Being Acknowledged
	uint32_t packNum;
	uint32_t len;
	uint32_t bytes;
};

//Send Method For Go Back Protocol
int goBack(int cLength, struct sockaddr_in client, int sd, char *buf, int numPackets, int data_size, int bytes, int dropRate){

	printf("--------------------Inside Go-Back-N Protocol------------------------\n");
       	printf("Begin: \n");
	printf("Number Of Packets: %d Of Size: %dB\n", numPackets, data_size);

	// Modify the buffer to simulate dropped packets
	struct packet sPacket;
	struct ack sAck;
	struct timeval timeout;
 	struct timeval start, end;
	long d = 0;
	int m = 0;  // Size of modified buffer
	int i = 0;
	int j = 0;	
	int randNum = 0;
	int n = 0;   // i: index of original buffer, j: index of new buffer, r: random number deciding whether to drop packet
	int connection = 0;
	int dTime = 0;
	int numRetry = 0;
	int bytes_acked = 0;
	int bRemain = bytes;
	int last_ack = 0;
	int elapsed = 0;
	char dBuffer[numPackets];

	//Initialize Frame NUmber
	sPacket.packNum=0;


	//While Loop To Continuously Send Data
	while(1 == 1){

		//Set A Packet Processing Delay
		gettimeofday(&start, NULL);

		//While Loop To Send Packets Continuously
		while(1 == 1){

			//Loop To Send A Packet, Don't Send More Than We Want or Than The Window Allows
			if((sPacket.packNum < numPackets) && (sPacket.packNum - last_ack) < 5){
				sPacket.numPackets = numPackets;
            			//Check For Last Sent Packet
				if(sPacket.packNum+1 == numPackets){
					//Last Packet Could Have A Larger Data Size Then Length
               				sPacket.len = bytes % data_size;
           			 }else{
               				sPacket.len = data_size;
            			}

            			//Use Memcopy To Send Data
            			memcpy(sPacket.data, (buf+(sPacket.packNum*data_size)), sPacket.len );
				//Set Random Number To Drop Random Packets
				randNum = 1 + (rand() % 100);
            
				//If Our Random Number Is Higher Than Drop Rate, Begin Dropping Packets, If Not Send Packet
				if(randNum > dropRate){
					//Set Up Connection
               				connection = sendto(sd, &sPacket, sizeof(sPacket), 0, (struct sockaddr *)&client, cLength);

	       				//Check For Frame Send Error
               				if(connection == -1){
                  				//Fall Through Loop
               				}else{
						//Print Frame Number
                  				printf("Frame Number: %d\n", sPacket.packNum);
                  				sPacket.packNum++;
               				}
            			}else{
						//Else Drop Packet
               					j++;
                  				printf("Packet Dropped. It Was Packet Number: %d\n", sPacket.packNum);
            			}
         		}else{
            			printf("Window Full On Packet: %d\n", sPacket.packNum);
         		}


          		//Set Up Connection
          		connection = recvfrom(sd, &sAck, sizeof(sAck), MSG_DONTWAIT, (struct sockaddr *)&client, &cLength);

			//If Loop To Check Connection
          		if(connection == -1){
             			//If Connection Fails, Fall Through Loop
          		}else{
				//Else Receive Ack
				//If Loop To Receive Ack
             			if(sAck.packNum == last_ack+1){
                			// Receive Ack with Number of Bytes
                			last_ack = sAck.packNum;
                			bytes_acked += sAck.bytes; 
                			printf("ACK #:%d\n", sAck.packNum);

             			}else{
					//Else Ack Is Ignored
                			printf("ACK #: %d --> Ignored.\n", sAck.packNum);
				       	printf("Want ACK Number: %d\n", last_ack+1);
             			}
          		}

			//Stop The Delay, Get The Number Of Seconds      
         		gettimeofday(&end, NULL);
			elapsed = (end.tv_sec - start.tv_sec) * 1000;
			elapsed += ((end.tv_usec - start.tv_usec + 500) / 1000);
			//If Loop To Check Blocking, Either Break From Loop Or Wait Longer
         		if(elapsed > 1){
            			//If The Timer Is Over, Break From Loop
            			break;
         		}else{
            			//If The Timer Is Not Over, Sleep
            			usleep(0.25*1000*1000*1000);
         		}
       		}

      		//Go Back If Needed
       		printf("Checking If Need To Go Back...\n");

		//Loop To Check If We Have To Go Back Or Not
       		if(last_ack == numPackets){
			// Set Frame Number To The Current Number Of Frames, Set Negative Frame Length 
         		sPacket.packNum = numPackets;
         		sPacket.len = -1;

			//Set Up Connection
         		connection = sendto(sd, &sPacket, sizeof(sPacket), 0, (struct sockaddr *)&client, cLength );
	
	 		//Check For Error On FIN, Send Again If So
         		if(connection == -1){
            			//If Error, Fall Through Loop
         		}else{
				//If There Is No Error On The Last Packet
				//Print Success Message, Break From Loop
            			printf("Finished.\n");
				printf("Finished Frame #%d\n", sPacket.packNum);
            			break;
         		}
       		}else{
         		//If There Is More To Send, Send It
         		connection = sPacket.packNum - last_ack;
         		sPacket.packNum = last_ack;
         		printf("Going Back...%d\n", n);
       		}

	}

	//Print Final Stats, Return Number Of Bytes Sent and Checked
	printf("Finished.\n");
	printf("%d Sent Packets\n", numPackets);
	printf("Go Back Number: %d.\n", bytes_acked);
	return bytes_acked;
}

//Recieve Method For Go-Back
int getBack(int *cLength, struct sockaddr_in *client, int sd, char *buf, int *data_size, int dropRate){
   
	//Declare Necessary Variables
	struct packet sPacket;
	struct ack sAck;
	struct timeval timeout;
	int connection = 0;
	int i = 0;
        int packNum = -1;
	int numPackets = 1;
        int bReceived = 0;
	int randNum = 0;
	int dTime = 0;
	int numRetry = 0;
	int recCheck = 0;


	//Set The Next Frame
	sAck.packNum = 0;
	sAck.len = -1;


   	printf("-----------------------Receiving Go Back N------------------------------: \n");

   	//While Loop To Continuously Loop To Receive Packets
    	while(1 == 1) {

      		//Check For Packet
      		if(sAck.packNum <= 0){
         		// Block While We Wait For A Client To Connect
         		recCheck=0;
      		}else{
         		// Otherwise, Don't Block
         		recCheck=MSG_DONTWAIT;
      		}

		//Set Up A Receiver
      		connection = recvfrom(sd, &sPacket, sizeof(sPacket), recCheck, (struct sockaddr *)client, cLength);

      		if(connection == -1){
         		//If We Can't Start Receiving, Just Pass Through Loop
         
      		}else{
         		//Only Set The Timeout once, and make sure to set it after we receive the first FRAME
			//If Loop To Receive Each Packet
         		if(sPacket.packNum == sAck.packNum){
            			//If Frame Is Correct
            			*data_size = max( *data_size, sPacket.len );
           			 numPackets = sPacket.numPackets;
            				if(sPacket.len == -1){
               					//If No Frames Left, Print Finished Message
               					//printf("Finished. #%d\n", sPacket.packNum);
               					printf("Finished. Received %d Packets", sPacket.numPackets);
               					sAck.bytes = sPacket.len;
               					sAck.packNum++;
            				}else{
						//Else Keep Receiving Pakets
               					printf("Receiving Packet #%d\n", sPacket.packNum);
               					//Use Memcopy To Get Data
               					memcpy((buf+(sPacket.packNum*(*data_size))), sPacket.data, sPacket.len);
               					sAck.bytes = sPacket.len;
               					bReceived += sPacket.len;
						sAck.packNum++;
            				}

            			// Fall through to the ACK code block
         		}else{
            			printf("Packet Number: %d --> Ignored. Wanted Packet Number: %d\n", sPacket.packNum, sAck.packNum);
            			continue;
         		}
      		}

		//Send an ACK
       		//Set Random Number To Randomly Drop Packets
        	randNum = 1 + (rand() % 100);

        	if(randNum > dropRate){
           		// Send the ACK
           		connection = sendto(sd, &sAck, sizeof(sAck), 0, (struct sockaddr *)client, *cLength);
			
			//Check If Send Worked
           		if(connection == -1){
              			//Fall Through The Loop
           		}else{
              			printf("ACK\n");
           		}
        	}else{
           		printf("ACK --> Dropped.\n");
		}

		//If Run Out Of Frames Or More ACKS than Frames Sent, Break From Loop
		if( numPackets == -1 || sAck.packNum > numPackets) {
        		 break;
       		}//End If
	}//End While

	printf("Finished.\n");
       	printf("Received %d packets.\n", numPackets);
       	printf("Total Bytes Sent: %d B\n", bReceived);
	
	//Return The Total Bytes Received
	return bReceived;
}//End Receive GoBackN

/////////////////////////////////////////////////-----------Stop and Wait Protocol------------/////////////////////////////////////////////////////////////////////
int stopWaitSend(int cLength, struct sockaddr_in client, int sd, char *buf, int numPackets, int dataSize, int bytes, int dRate){

	//Print Starting Message
        printf("----------------------Inside Stop And Wait Protocol-------------------------\n");
	printf("Begin: \n");
	printf("Number Of Packets: %d Of Size: %dB\n", numPackets, dataSize);

        //Declare Necessary Variables
        struct packet sPacket;
        struct ack sAck;
        struct  timeval start, end;
        struct timeval timeout;
        long d;
        int m = 0;
        int randNum = 0;
        int connection = 0;
        int dTime = 0;
        int numRetry = 0;
        int bSent = 0;
        int bRemain = 0;
        int mOffset = 0;
        int mBytes = 0;
        char dBuffer[numPackets];

        //Set A Timeout
        timeout.tv_sec = 2;
	timeout.tv_usec = 0;
        
	// Randomize Timeout
        randNum = 1 + (rand() % 2);
        if(randNum == 0){
                timeout.tv_sec++;
        }else{
                timeout.tv_sec--;
        }

        timeout.tv_usec = 0;
        
	//Set A Timeout
        if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout) ) < 0){
                return -1;
        }
	
	//Set Frame Number, Number Of Frames, Bytes Remaining, And Number Of Retries
        sPacket.packNum = 0;
        sPacket.numPackets = numPackets;
        bRemain = bytes - bSent;
        numRetry = 0;

	//While Loop To Send
        while(1 == 1){
		//If Loop To Set Length Of Packet By Checking If Frame Number Is Equal To Number Of Frames
                if(sPacket.packNum+1 == numPackets){
                        sPacket.len = bytes % dataSize;
                }else if(sPacket.packNum+1 < numPackets){
                        sPacket.len = dataSize;
                }else{
                        sPacket.len = 0;
                }

                //Set A Delay Measurement
                gettimeofday(&start, NULL);

		//If Frame Number Is Less Than Number Of Frames, Calculate The Buffer Offset, New Byte Count, And Use Memcopy To  Send Data
                if(sPacket.packNum < numPackets){
                        mOffset = sPacket.packNum * dataSize;
                        mBytes = sPacket.len;
                        memcpy(sPacket.data, (buf+mOffset), mBytes );
                }else{
			//If We No Longer More Frames To Send, Finish.
                        printf("Finished. Sent: %d Packets\n", numPackets);
                }

                //Randomize Drop Rate
                randNum = 1 + (rand() % 100); // range of 1-100

		//If Loop To Handle Randomly Dropped Packets
                if(randNum > dRate){
                        //Set Connection To Send Packet
                        connection = sendto(sd, &sPacket, sizeof(sPacket), 0, (struct sockaddr *)&client, cLength );

			//If Connection Fails
                        if(connection == -1){
				//Depending On The Number Of Retries, Try Again, Or Return
                                if(numRetry < 20){
					//If Retrying, Increment Retries And Continue
                                        numRetry++;
                                        continue;
                                }else{
                                        return -1;
                                }
                        }

			//If Our Packet Number Is Less Than The Number Of Packets Left
                        if(sPacket.packNum < numPackets){
                                //Send Packet, Increment Bytes Sent
                                printf("Sent Packet Number: %d\n", sPacket.packNum);
                                bSent += sPacket.len;
                        }else{
                                printf("Finished Packet Number: %d\n", sPacket.packNum);
                        }
                }else{
			//If Packet Number Is Less Than Number Of Packets Left, Drop Packet
                        if(sPacket.packNum < numPackets){
                                printf("Packet Number: %d --> Dropped\n", sPacket.packNum);
                        }else{
                                 printf("Finished Packet Number: %d --> Dropped\n", sPacket.packNum);
                        }
                }

                //If Our Frame Number Is Equal To Number Of Frames Sent, Wait For Ack
                if(sPacket.packNum == numPackets){
                        break;

                }else{
			//Set A Connection
                        connection = recvfrom(sd, &sAck, sizeof(sAck), 0, (struct sockaddr *)&client, &cLength);

			//If Connection Fails, Set Up Delay And Retry 30 Times
                        if(connection == -1){

                                if(sPacket.packNum < numPackets){
					//Set Up Delay
                                        gettimeofday(&end, NULL);
                                        dTime = (end.tv_sec - start.tv_sec) * 1000;
                                        dTime += ((end.tv_usec - start.tv_usec + 500) / 1000);
					//If 50 Is Less Than The Delay Time
                                	if(50 < dTime) {
						//Retry Send
                                        	if(numRetry < 30) {
                                                	//Increment Number Of Retries
                                                	numRetry++;
                                                	continue;
                                        	}else{
							//If Too Many Failed Retries, Return
                                                	return -1;
                                        	}
                                	}else{
                                        	//Attempt To Retry Again
                                        	if(numRetry < 30) {
                                                	numRetry++;
                                                	continue;
                                        	}else{
							//If Too many Failed Retries, Return
							return -1;
						}
					}
				}
                        }

                        //If Loop To Check If Ack Was Sent
                        if(sAck.len == -1){
				//If Ack Number Not The Same As Packet Number
                                if(sAck.packNum != sPacket.packNum){
					//If Ack Number Is At Second To Last Position
                                        if(sAck.packNum == sPacket.packNum -1 ){
						//Set Packet Number To The Ack Number
                                                sPacket.packNum = sAck.packNum;
                                        }else{
                                                //Retry Failure
                                                return -1;
                                        }
                                }
                        }
			//Increment The Packet Number
                        sPacket.packNum++;
                        //Re-Initiallize The Retry Number
			numRetry = 0;

			//Check For Same Amount Of Acks And Packets, If Equal, We're Done
                        if(sAck.packNum >= numPackets){
                                printf("Finished Packet Number: %d\n", sAck.packNum);
                                break;
                        }

                        printf("Sent ACK Number: %d\n", sAck.packNum);
                }

        }

	//Print Finished Message And Return Bytes Sent
        printf("Finished.\n");
	printf("Packets Sent: %d\n", numPackets);
        return bSent;

}//End Send Method


//Stop And Wait Receive Method
int stopWaitReceive(int *cLength, struct sockaddr_in *client, int sd, char *buf, int *dataSize, int dRate){

        //Declare Necessary Variables
        struct packet sPacket;
        struct ack sAck;
        struct timeval timeout;
        int connection = 0;
        int i = 0;
        int packNum = -1;
        int numPackets = 1;
        int bReceived = 0;
        int randNum = 0;
        int pIndex = -1;
        int dTime = 0;
        int numRetry = 0;
        int mOffset = 0;
        int mBytes = 0;

	//Print Message We Are Receiving Packets
        printf("Stop And Wait: Receiving...\n");

	//Set Frame Number
        sPacket.packNum = -1;

	//While We Still Have Frames To Send
        while(sPacket.packNum+1 <= numPackets){

		//Set Up A Connection
                connection = recvfrom(sd, &sPacket, sizeof(sPacket), 0, (struct sockaddr *)client, cLength);

		//If We Have A Connection
                if(connection == -1){
			//If We Have Retried More Than 30 Times, Break From Loop
                        if(numRetry > 20){
                                break;
                        }
			//Increment Retry Counter If We Are Still Under 30, Then Continue
                        numRetry++;
                        continue;
                }

		//If Our Packet Length Is -1 Then Set The Frame Number To -1 And Continue
                if( sPacket.len == -1 ){
                        sPacket.packNum = -1;
                        continue;
                }

		//If We Still Have Frames To Receive
                if(sPacket.packNum < numPackets){
                        printf("Receiving Packet Number: %d\n", sPacket.packNum);
                }else{
                        printf("Receiving Finish Number: %d\n", sPacket.packNum);
                }

		//Set Frame Number, Data Size, And Number Of Packets Left
                packNum = sPacket.packNum;
                *dataSize = max( *dataSize, sPacket.len);
                numPackets = sPacket.numPackets;

                //Set The Ack Number To The Packet Number, Set Ack Length
                sAck.packNum = sPacket.packNum;
                sAck.len = -1;

                //Set Random Drop Rate
                randNum = 1 + (rand() % 100);

		//If We Do Not Ramdomly Drop Packet
                if(randNum > dRate){

                        //Create A Connection And Send packet
                        connection = sendto(sd, &sAck, sizeof(sAck), 0, (struct sockaddr *)client, *cLength);
                        
			//Check If Our Connection Is Working
			if(connection == -1){
                                continue;
                        }

			//If We Still Have Packets To Receive
                        if(sPacket.packNum < numPackets){
                                printf("Received Packet ACK Number: %d\n", sAck.packNum);
                        }else{
                                printf("Received Packet Finish Number: %d\n", sAck.packNum);
                        }
                }else{
			//If We Dropped A Packet And We Stll Have Packets To Send, Print Dropped Message
                        if(sPacket.packNum < numPackets){
                                printf("Received Packet Ack Number: %d --> Dropped\n", sAck.packNum);
                        }else{
                                printf("Received Packet Finish Number: %d --> Dropped\n", sAck.packNum);
                        }
                        continue;
                }
		//If The Packet Number Is Less Then The Number Of Packets
		if(sPacket.packNum < numPackets){
			//If The Packet Index And Frame Number Aren't The Same
			if(pIndex != sPacket.packNum){
				//Set The Buffer Memory Offset, Bytes
				//Use Memcopy To Receive What Is In The Buffer, Increment Bytes Received, And Increment Packet Index
                                mOffset = (sPacket.packNum*(*dataSize));
                                mBytes = sPacket.len;
                                memcpy((buf+mOffset), sPacket.data, mBytes);
                                bReceived += sPacket.len;
                                pIndex = sPacket.packNum;
                        }
                }
		//Reset Number Of Retries
                numRetry = 0;
        }	
	//Print Final Received Messgae
        printf("Finished Receiving.\n");
       	printf("Received: %d Packets\n", numPackets);
        return bReceived;
}
