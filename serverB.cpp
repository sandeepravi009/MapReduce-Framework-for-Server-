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
#include <sys/wait.h> 

#define SUM 0
#define MIN 1
#define MAX 2
#define SOS 3

//#define MY_DEBUG
#define MAX_RECBUF_SIZE_BYTES 1600
#define MAX_RECBUF_ELEMENTS 400
#define SERVER_UDP_PORT 22674
char ser = 'B';

int udp_sock,clientAddrLen,recLen,totalNums;
long process(int op,int* recBuf,int totalNums);
int SetupUDP();
int recBuf[MAX_RECBUF_ELEMENTS];

struct sockaddr_in server,client,BackEndServer;

int main()
{//			for(i=1;i<=totalNums;i++)
//
	
	int i=0,k,op;	
	long output;
	char outputStr[50];
	memset(recBuf,0,MAX_RECBUF_ELEMENTS);
	udp_sock = SetupUDP();
	clientAddrLen = sizeof(struct sockaddr);	
	while(1)
	{
		
		recLen=recvfrom(udp_sock,recBuf,MAX_RECBUF_SIZE_BYTES,0,(struct sockaddr*)&client,(socklen_t*)&clientAddrLen);
				
		if(recLen>0)
		{
			totalNums = recLen/sizeof(int)-1;			// -1 to remove op on top
			printf("The Server %c has received %d numbers\n",ser,totalNums);
			if(!strcmp((char*)recBuf,"sum"))
			{
				op = SUM;
			}
			if(!strcmp((char*)recBuf,"min"))
			{
				op = MIN;
			}
			if(!strcmp((char*)recBuf,"max"))
			{
				op = MAX;
			}
			if(!strcmp((char*)recBuf,"sos"))
			{
				op = SOS;
			}
			
			#ifdef MY_DEBUG
			for(i=1;i<=totalNums;i++)
				printf("%d -> %d\n",i,recBuf[i]);
			#endif
			
			#ifdef MY_DEBUG
				printf("recBuf[1] is pointing to: %d\n",&recBuf[1]);
			#endif
			
			output = process(op,&recBuf[1],totalNums);
			
			#ifdef MY_DEBUG
				printf("output: %d\n",output);
			#endif
			
				
			k = sendto(udp_sock,&output,sizeof(long),0,(struct sockaddr*)&client,(socklen_t)clientAddrLen);
			
			if(k<0)
			{
				perror("Server A: sending ACK error: \n");
			}
			printf("The Server %c has successfully finished sending the reduction value to AWS server\n",ser);
			
		}
		
	}
	return 0;
}

long process(int ch,int *vals,int size)
{
	long output=0,temp=0;
	int i;
	char processStr[4];
	
	#ifdef MY_DEBUG
		printf("Now made output1 as int\n");
		printf("vals is pointing to: %d\n",vals);
		printf("initial value of output1: %d\n",output1);
	#endif
	
		switch(ch)
	{
		case SUM:
			strcpy(processStr,"SUM");
			for(i=0;i<size;i++)
				output += *(vals+i);
		break;
			
		case MIN:
			strcpy(processStr,"MIN");
			//processStr[4] = 'MIN\0';
			output = *(vals);
			for(i=0;i<size;i++)
				if(output>*(vals+i))
					output = *(vals+i);	
		
		break;
		
		case MAX:
			strcpy(processStr,"MAX");
			//processStr[4] = 'MAX\0';
			output = *(vals);
			for(i=0;i<size;i++)
				if(output<*(vals+i))
					output = *(vals+i);	
		
		break;
		
		case SOS:
			strcpy(processStr,"SOS");
			
			for(i=0;i<size;i++)
				output += ((*(vals+i))*(*(vals+i)));
		
		break;
		default:
			printf("Not valid operator\n");
		break;
			
	}
	
	
	printf("The Server %c has successfully finished the reduction %s: %d\n",ser,processStr,output);
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
	
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_UDP_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero,8);
	
	len = sizeof(struct sockaddr_in);
	setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	
	if((bind(udp_sock,(struct sockaddr*)&server,len))<0)
	{
		perror("UDP-bind: ");
		exit(-1);
	}
	printf("The Server %c is up and running using UDP on port %d.\n",ser,SERVER_UDP_PORT);
	return udp_sock;
	
}