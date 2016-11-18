	/*Title: Server-Client*/
/*Author: Sandeep Ravi*/
/*Date: 3-Oct-2016*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/wait.h> 

//#define MY_DEBUG
#define MAX_RECBUF_SIZE_BYTES 4400
#define MAX_RECBUF_ELEMENTS 1100
#define AWS_UDP_PORT 24674
#define AWS_TCP_PORT 25674
#define SUM 0
#define MIN 1
#define MAX 2
#define SOS 3

struct sockaddr_in aws_server,client,aws_client,BackEndServer;
unsigned int CliAddrlen;
int tcp_sock,udp_sock,cli;
long out_arr[4];
char genStr[3][200];
int SetupTCP();
int SetupUDP();
void Send2BackEnd(int servNo,int* val,int bytes,char* op);
char* strupr(char* p);
void MakeBufferAndSend(int* buf,int);
long processData(long vals[], char*proc);
int main()
{
	int sent,i,recLen,totalNums;
	char c;
	signed long val;
	
	int sp,ep;
	int recBuf[MAX_RECBUF_ELEMENTS];
//	char sam[100]="9288\r\n";
//	sscanf(sam,"%d",&val);
	memset(recBuf,0,sizeof(recBuf));
	CliAddrlen = sizeof(struct sockaddr_in);
	tcp_sock = SetupTCP();
	udp_sock = SetupUDP();
		
	printf("AWS Server up and running...\n");
	while(1)
	{
		#ifdef MY_DEBUG
			printf("Waiting for Client....\n");
		#endif
		if((cli = accept(tcp_sock,(struct sockaddr*)&client,&CliAddrlen))==-1)
		{
			perror("Accept: ");
			
		}
		printf("\nConnected to %s on port %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		
		recLen=1;
		while(recLen)
		{
			#ifdef MY_DEBUG
				printf("LOG:  while(recLen)\n");
			#endif
			recLen=recv(cli,recBuf,MAX_RECBUF_SIZE_BYTES,0);
					
			if(recLen>0)
			{
				totalNums = recLen/sizeof(int);		//this includes the operator
				printf("The AWS has received %d numbers from the client using TCP over port %d.\n",totalNums-1,AWS_TCP_PORT);
				MakeBufferAndSend(recBuf,totalNums);
				#ifdef MY_DEBUG 
					for(i=0;i<totalNums;i++)
						printf("%d-> %d\n",i,recBuf[i]);
				#endif
								
			}
		}
		#ifdef MY_DEBUG
		printf("Client Disconnected\n");
		#endif
	}
	
	c = getchar();
	return 0;
}
		
void MakeBufferAndSend(int* buf,int len)
{
	char op[4];
	int smallBuf[334],partition,i,sizeBytes;
	sscanf((char*)buf,"%s",op);
	#ifdef MY_DEBUG
		printf("Extracted op: %s\n",op);
		printf("Extracted op(hex): %04x\n",op);
	#endif
	op[3]='\0';

	memset(smallBuf,0,sizeof(smallBuf));

	memcpy(smallBuf,op,4);
	strupr(op);
	partition = (len-1)/3;		// -1 to remove op at ths top // elements in one partition
	sizeBytes = (partition+1)*sizeof(int);	//we cannot divide len by 3 as it is a even no. 
	memset(out_arr,0,sizeof(out_arr));
	memset(genStr,0,sizeof(genStr));
	out_arr[2]='$'; 	// special character to know if I got output from Server C
	for(i=0;i<3;i++)
	{
		#ifdef MY_DEBUG
			printf("\nAt MakeBufferAndSend i:%d op:%s\n",i,op);
			
		#endif
		memcpy(&smallBuf[1],(&buf[1])+(i*partition),partition*sizeof(int));  		
		Send2BackEnd(i,smallBuf,sizeBytes,op);
		#ifdef MY_DEBUG
			printf("After MakeBufferAndSend i:%d op:%s\n",i,op);
			
		#endif
		memset(&smallBuf[1],0,(partition*sizeof(int)));
		#ifdef MY_DEBUG
			printf("After memset(&smallBuf[1],0,sizeBytes = %d; i:%d op:%s\n\n",sizeBytes,i,op);
			
		#endif
		
	}
}		

void Send2BackEnd(int servNo,int* val,int bytes,char* op)
{
	struct sockaddr_in from;
	struct hostent *hostDetails;
	int addrStructSize,noBytesSent,noBytesRecv,BytesSent,error,ret;
	long out;
	int length;
	int RecvBuffer_BackEndServ=0;
	char c,serverName;
		
	socklen_t len1 = sizeof(int);
	length = sizeof(struct sockaddr_in);
	addrStructSize = sizeof(struct sockaddr_in);
	
	#ifdef MY_DEBUG
		printf("Entered Send2BackEnd with value of op being %s\n",op);
	#endif
	if((hostDetails = gethostbyname("localhost"))==0)	// this would fill the structure in hostDetails
		perror("Unknown Host\n");
	
	BackEndServer.sin_family=AF_INET;
	bcopy((char*)hostDetails->h_addr,(char*)&BackEndServer.sin_addr,hostDetails->h_length);	//the socket needs IP addr
	
	switch(servNo)
	{
		case 0:
			serverName = 'A';
			BackEndServer.sin_port=htons(21674);
			break;
		
		case 1:
			serverName = 'B';
			BackEndServer.sin_port=htons(22674);
			break;
		
		case 2:
			serverName = 'C';
			BackEndServer.sin_port=htons(23674);
			break;
		default:
		break;
	}
		
		noBytesSent=sendto(udp_sock,val,bytes,0,(struct sockaddr*)&BackEndServer,addrStructSize);
		
		if(noBytesSent < 0) 
			perror("ending failed\n");
		else
		{
			printf("The AWS has sent %d numbers to Backend-Server %c.\n",(noBytesSent-1)/4,serverName);
		}
		
		noBytesRecv=recvfrom(udp_sock,&RecvBuffer_BackEndServ,sizeof(long),0,(sockaddr*)&from,(socklen_t*)&length);
		
		if(noBytesRecv < 0)
			perror("recv Error..\n");
		else
		{
			#ifdef MY_DEBUG
				printf("%d -> val of op: %s and addr: %d\n",servNo,op,&op);
			#endif
			
			sprintf(&genStr[servNo][0],
					"The AWS received reduction result of %s from Backend-Server %c using UDP over port %d and it is %d",
					op,serverName,ntohs(from.sin_port),(long)RecvBuffer_BackEndServ);
			
			#ifdef MY_DEBUG
				printf("After sprintf %d -> val of op: %s and addr: %d\n",servNo,op,&op);
			#endif
			
			out_arr[servNo]=RecvBuffer_BackEndServ;
			#ifdef MY_DEBUG
				printf("out_arr: %d %d %d\n",out_arr[0],out_arr[1],out_arr[2]);
			#endif 
			if(out_arr[2]!='$')
			{
				#ifdef MY_DEBUG
					printf("No $\n");
				#endif
				printf("%s\n",genStr[0]);
				printf("%s\n",genStr[1]);
				printf("%s\n",genStr[2]);
				// then process the outputs
				out=processData(out_arr,op);
				//sprintf(genstr,"%d\0",&out);
				#ifdef MY_DEBUG
					
					printf("Output to Client: %d\n",out);
				#endif
				printf("The AWS has successfully finished the reduction %s: %d\n",op,out);
				#ifdef MY_DEBUG
					printf("Now trying to send to to Client\n");
					printf("tcp_sock: %d long size: %d\n", tcp_sock,sizeof(long));
				#endif
				
				BytesSent = send(cli,&out,sizeof(long),0);//,(struct sockaddr*)&client,(socklen_t)CliAddrlen);
				
				#ifdef MY_DEBUG
					printf("BytesSent: %d\n",BytesSent);
				#endif
				if(BytesSent>0)
				{
					printf("The AWS has successfully finished sending the reduction value to client.\n");
				}
				else
				{
					printf("Sent %d bytes\n",BytesSent);
					perror("AWS->Client:");
				}
			}
				
		}
		
		
		
}		
		
long processData(long vals[], char*proc)
{
	int ope=9,i,size=3;
	long output=0;
	#ifdef MY_DEBUG
		printf("Entered processData\n");
	//	printf("proc Data 1st element: %s\n",*proc);
	#endif 
	if(!strcmp((char*)proc,"SUM") || !strcmp((char*)proc,"sum"))
	{
		ope = SUM;
	}
	if(!strcmp((char*)proc,"MIN") || !strcmp((char*)proc,"min"))
	{
		ope = MIN;
	}
	if(!strcmp((char*)proc,"MAX") || !strcmp((char*)proc,"max"))
	{
		ope = MAX;
	}
	if(!strcmp((char*)proc,"SOS") || !strcmp((char*)proc,"sos"))
	{
		ope = SOS;
	}
	#ifdef MY_DEBUG
		printf("op: %d\n", ope);
	#endif
	switch(ope)
	{
		case SUM:
			for(i=0;i<size;i++)
				output += *(vals+i);
		break;
			
		case MIN:
			output = *(vals);
			for(i=0;i<size;i++)
				if(output>*(vals+i))
					output = *(vals+i);	
		
		break;
		
		case MAX:
			output = *(vals);
			for(i=0;i<size;i++)
				if(output<*(vals+i))
					output = *(vals+i);	
		
		break;
		
		case SOS:
			for(i=0;i<size;i++)
				output += *(vals+i); // for SOS we need to just add it
		
		break;
		default:
			printf("Not valid operator\n");
		break;
			
	}
	return output;
	
}
int SetupUDP()
{
	int udp_sock=0,len=0,yes=1;
	if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0))== -1)
	{
		perror("UDP-socket: ");
		exit(-1);
	}
	
	aws_client.sin_family = AF_INET;
	aws_client.sin_port = htons(AWS_UDP_PORT);
	aws_client.sin_addr.s_addr = INADDR_ANY;
	bzero(&aws_client.sin_zero,8);
	
	len = sizeof(struct sockaddr_in);
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	if((bind(udp_sock,(struct sockaddr*)&aws_client,len))<0)
	{
		perror("UDP-bind: ");
		
	}
	#ifdef MY_DEBUG
				printf("LOG:  SetupUDP()\n");
	#endif
	return udp_sock;
	
}
int SetupTCP()
{
	int sock,len=0,yes=1;
	if((sock = socket(AF_INET, SOCK_STREAM, 0))== -1)
	{
		perror("socket: ");
		
	}
	#ifdef MY_DEBUG
		printf("Setup TCP sock: %d\n",sock);
	#endif
	
	aws_server.sin_family  = AF_INET;
	aws_server.sin_port = htons(25674);
	aws_server.sin_addr.s_addr = INADDR_ANY;
	bzero(&aws_server.sin_zero,8);
	
	len = sizeof( struct sockaddr_in);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	if((bind(sock,(struct sockaddr*)&aws_server,len)))
	{
		perror("bind: ");
		
	}
	if((listen(sock,5) == -1))
	{
		perror("listen ");
		
	}
	#ifdef MY_DEBUG
				printf("LOG:  SetupTCP()\n");
	#endif
	return sock;
}

char* strupr(char* p)
{
	unsigned char* q;
	q = (unsigned char*) p;
	while(*q)
	{
		*q = toupper(*q);
		q++;
	}
	return p;
}
