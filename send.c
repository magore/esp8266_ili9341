#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int	uint32_t;

//#define TCP_PORT 31416
#ifndef TCP_PORT
#error Please define TCP_PORT
#endif

int escape(char *message, int size)
{
	char *buffer;
	char *bptr;
	char *mptr;
	int len;
	unsigned char c;

	len = strlen(message);
	if(len > size)
	{
		fprintf(stderr,"message too long\n");
		len = size;
	}

	mptr = message;
 	buffer = calloc(len+2,1);
	bptr = buffer;
	if(!buffer)
	{
		perror("Can not allocate buffer\n");
		exit(1);
	}

	while(*mptr)
	{
		c = *mptr++;
		if(c == '\\')
		{
			if(*mptr)
			{
				c = *mptr;
				if( c == 'n')
				{
					c = '\n';
					mptr++;
				}
				else if( c == 'r')
				{
					c = '\r';
					mptr++;
				}
				else if( c == 't')
				{
					c = '\t';
					mptr++;
				}
				else if( c == 'f')
				{
					c = '\f';
					mptr++;
				}
			}
		}
		*bptr++ = c;
	}
	*bptr = 0;
	strcpy(message,buffer);
	return(strlen(message));
}

void send_message(char *message, char *ip, int port, int echoback)
{
    int sockfd, portno, n;
	char buffer[4096];

    struct sockaddr_in dest_addr;
    struct hostent *server;

    portno = port;
	// IF UDP use: AF_INET, SOCK_DGRAM, IPPROTO_UDP

	// TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) 
	{
        perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(ip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	printf("Host name: %s\n", server->h_name);

    bzero((char *) &dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&dest_addr.sin_addr.s_addr,
         server->h_length);
    dest_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&dest_addr,sizeof(dest_addr)) < 0) 
	{
        perror("ERROR connecting");
		exit(1);
	}
    n = write(sockfd,message,strlen(message)+1);
    if (n < 0) 
	{
        perror("ERROR writing to socket");
		exit(1);
	}
	
	// reads characters coming back from the connection - debugging
    // interrup to exit
    // this code blocks waiting for input
	if(echoback)
	{
		do
		{
			n = read(sockfd,buffer,sizeof(buffer)-1);
			write(1, buffer, n);
		}
		while(n);
	}

	close(sockfd);
}


int main(int argc,char *argv[])
{
	int i,ret;
	int echoback = 0;
	struct net *p;
	char message[8192];
	char ip[INET_ADDRSTRLEN];
	int port_no = TCP_PORT;

// htonl() and htons()..

	memset(ip,0,sizeof(ip)-1);
	memset(message,0,sizeof(message)-1);
	for(i=1;i<argc;++i) {
		if(strcmp(argv[i],"-i") == 0) {
       		strcpy(ip, argv[++i]);
		}
		if(strcmp(argv[i],"-m") == 0) {
       		strncpy(message,argv[++i], sizeof(message)-1);
		}
		if(strcmp(argv[i],"-p") == 0) {
       		sscanf(argv[++i], "%d", &port_no);
		}
		if(strcmp(argv[i],"-e") == 0) {
       		echoback = 1;
		}
	}
		
	if(!strlen(ip) || !strlen(message))
	{
		fprintf(stderr,"usage: -i ip_address -m \"message\" [ -p port ]\n ");
		exit(1);
	}
	if(!strlen(message))
	{
		fprintf(stderr,"Message missing\n");
		exit(1);
	}
	escape(message, sizeof(message)-1);
	printf("ip:%s, port:%d, message:\n%s\n",ip,port_no,message);
	fflush(stdout);
	send_message(message, ip, port_no, echoback);
	return(0);
}
