#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/wait.h> 
#include <sys/ioctl.h>

//#define MY_DEBUG
#define MAX_RECBUF_SIZE 1000

char* strupr(char* p);


int main(int argc, char **argv)
{
	int status;
	struct addrinfo hints;
	struct addrinfo* res,*p;
	struct sockaddr_in me_clientinfo;
	struct sockaddr_in* me_clientinfo_ptr;
	socklen_t me_clientinfo_len;
	FILE * filePtr;
    long FileSize;//out;
	long out;
	char c,strDelim[2]="\n",*token;
	char *Buf;
	struct sockaddr_in server,client;
	int sock,cli,sent,i,val,error,ret,iMode=0;
	int nums[1000],k,q,size;
	char oper[4];
	socklen_t len = sizeof(struct sockaddr_in);
	
	me_clientinfo_ptr = &me_clientinfo;

	memset(nums,0,1000*sizeof(int));
	if(argc<1)
	{
		printf("Please enter argument..\n");
		exit(1);
	}
	
	if(!strcmp(argv[1],"sum"))
	{
		memcpy(oper,"SUM\0",4);
		memcpy(nums,"sum\n",4);
	}
	if(!strcmp(argv[1],"max"))
	{
		memcpy(oper,"MAX\0",4);
		memcpy(nums,"max\n",4);
	}
	if(!strcmp(argv[1],"min"))
	{
		memcpy(oper,"MIN\0",4);
		memcpy(nums,"min\n",4);
	}
	if(!strcmp(argv[1],"sos"))
	{
		memcpy(oper,"SOS\0",4);
		memcpy(nums,"sos\n",4);
	}
		
	filePtr = fopen ("nums.csv","r");
	if (filePtr==NULL) 
		perror ("Error opening file");
	else
	{
		fseek (filePtr, 0, SEEK_END);   // this points to the end of the file
		FileSize=ftell (filePtr);			// number of bytes present till the loc of FilePtr
		
		#ifdef MY_DEBUG
		printf ("Size of nums.csv: %ld bytes.\n",FileSize);
		#endif
		
		fseek (filePtr, 0 ,SEEK_SET);

		Buf = (char*)malloc(FileSize+1);
		fread(Buf,FileSize,1,filePtr);
		fclose(filePtr);
		Buf[FileSize]=0;
		
		token = strtok(Buf,strDelim);
		i=1;						// i=0 is placeholder for operator
		while(token!=NULL)
		{
			sscanf(token,"%d",&nums[i]);
			token=strtok(NULL,strDelim);
			i++;
		}
		free(Buf);
		size=i*sizeof(int);
		
	}
	
	
	if((sock = socket(AF_INET, SOCK_STREAM, 0))== -1)
	{
		perror("socket: ");
		exit(-1);
	}
		
	server.sin_family  = AF_INET;
	server.sin_port = htons(25674);
	
	
	printf("Client is up and running\n");

	
	if(connect(sock,(struct sockaddr*)&server,sizeof(server)) < 0)
	{
		perror("Connecting problem");

	}	
	
	#ifdef MY_DEBUG
	printf("Connected to server on port %d\n",ntohs(server.sin_port));
	#endif
	
	if((k=send(sock,nums,size,0))<0)
	{
		printf("Client sending error\n");
	}
	else
	{
		printf("The client has sent the reduction type %s to AWS.\n",oper);
		printf("The client has sent %d numbers to AWS.\n",(k-1)/4);
	
	}
	#ifdef MY_DEBUG
		printf("LOG:  now at RECV\n");
	#endif
			
		if((q=recv(sock,&out,sizeof(long),0))<0)//,(struct sockaddr*)&me_clientinfo,&me_clientinfo_len))<=0)
		{
			perror("client recv: ");
		}
		else
			printf("The client has received reduction %s: %d \n",oper,out);
	
	close(sock);
	close(sock);
	
	return 0;
}

char* strupr(char* p)
{
	unsigned char* q;
	q = (unsigned char*) p;
	while(*q)
	{
		*q = tolower(*q);
		q++;
	}
	return p;
}
