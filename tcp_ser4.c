/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd);                                                        // transmitting and receiving function

int main(void)
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;

//	char *buf;
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);                              //listen
	if (ret <0) {
		printf("error in listening");
		exit(1);
	}

	while (1)
	{
		printf("Waiting to receive window size\n");
		sin_size = sizeof (struct sockaddr_in);
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
		if (con_fd <0)
		{
			printf("error in accept\n");
			exit(1);
		}

		if ((pid = fork())==0)                                         // creat acception process
		{
			close(sockfd);
			str_ser(con_fd);                                          //receive packet and response
			close(con_fd);
			exit(0);
		}
		else close(con_fd);                                         //parent process
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	char recvw[DATALEN];
	struct ack_so ack;
	int end, n = 0;
	int windowsize = 0;
	int datasize = 0;
	int packetreceived = 0 ;
	long lseek=0;
	int mode = 0;
	end = 0;

	printf("receiving data!\n");

	while(!end)
	{
		if(mode ==0){
			if((n= recv(sockfd, &recvw, DATALEN, 0))==-1)
			{
				printf("receiving error!\n");
				return;
			}
			char *e;
			char d[BUFSIZE];
			int index;
			e = strchr(recvw, '-');
			index = (int)(e - recvw);
			strncpy(d,recvw + index+1 ,strlen(recvw)-index-1);
			datasize = atoi(d);
			printf("%s\n", d);
			recvw[n] = '\0';
			windowsize = atoi(recvw);
			if(windowsize<=0){
				printf("windowsize is less than or equal to zero");
				return;
			}
			ack.num = 1;
			ack.len = 0;
			if ((n = send(sockfd, &ack, 2, 0))==-1)//send the ack
			{
				printf("send error!");								
				exit(1);
			}
			memset(recvw, 0, sizeof(recvw));
			mode = 1;
			printf("the received number of windows is:%d\n", windowsize);

		}
		if(mode == 1)
		{
			while(packetreceived<windowsize && !end){
				if ((n= recv(sockfd, &recvs, DATALEN, 0))==-1)                                   //receive the packet
				{
					printf("error when receiving\n");
					exit(1);
				}
				memcpy((buf+lseek), recvs, n);
				packetreceived += 1;
				lseek += n;			
				if(lseek>=datasize)
				{
					end = 1;
					mode = 0;
				}
			}			
			printf("packet received: %d\n", packetreceived);
			packetreceived = 0;
			ack.num = 1;
			ack.len = 0;
			if ((n = send(sockfd, &ack, 2, 0))==-1)//send the ack
			{
				printf("send error!");								
				exit(1);
			}
		}
	}
	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
	
}
