
#include "Server.h"

int sockfd;  

Server S;
static char *LOGFILE = "request.log";    


void signalHandler(int sig)
{
	if(sig==SIGINT)
	{
		printf("\n---------END OF SERVER SIDE--------\nEXIT\n");
		S.logClose();
		close(sockfd);
		exit(EXIT_SUCCESS);
	}
}		


int main(void)
{
	struct info clue, *server_data, *result;
	int log;
	int bytes;
	struct sockaddr_store server_addr;
	char buffer[MAXLEN];
	socklen_t len;
	char dstination[INET6_ADDRSTRLEN];
	
	signal(SIGINT,signalHandler);

	// Configuration of server starts 
	memset(&clue, 0, sizeof clue);//sets the first count bytes of dest to the value c
	clue.ai_family = AF_UNSPEC; 
	clue.ai_socktype = SOCK_DGRAM;
	clue.ai_flags = AI_PASSIVE; 
	
	if ((log = getaddrinfo(NULL, MYPORT, &clue, &server_data)) != 0) 
	{
		S.logger("Server: getaddrinfo",'f',__func__,__LINE__);
		fprintf(stderr, "SERVER: GET INFORMATION--- %s\n", gai_strerror(log));
		return 1;
	}

	// Loop through all the results and binding to the first 
	for(result = server_data; result != NULL; result = result->ai_next) 
	{
		if ((sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1) //socket creation
		{
			S.logger("---Server socket---",'w',__func__,__LINE__);
			perror("---SERVER socket---");
			continue;
		}
		if (bind(sockfd, result->ai_addr, result->ai_addrlen) == -1) 
		{
			S.logger("---Server socket---",'w',__func__,__LINE__);
			close(sockfd);
			perror("---SERVER bind Success---");
			continue;
		}
		break;
	}
	if (result == NULL) 
	{   S.logger("ERROR ---> Server failed to bind",'f',__func__,__LINE__);
    
		fprintf(stderr, "ERROR---> SERVER failed to bind socket\n");
		return 2;
	}
	freeinfo(server_data);
	S.Openlog(LOGFILE);
	
	printf("SERVER--> Geting data from receiver...\n");
	// Configuration of server ends 
	

	//Main implementation starts 
	// Waiting for the first request from client - RRQ/WRQ 
	// Iterative server implementation 
	while(1)
	{
		memset(&server_addr,0,sizeof(server_addr));
		addr_len = sizeof erver_addr;
		if ((bytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0, (struct sockaddr *)&server_addr, &addr_len)) == -1) 
		{
			S.logger("Server: recvfrom",'f',__func__,__LINE__);
			S.errorHandler(bytes,"SERVER: recvfrom");
		}
		S.logger("Server got the packet",'i',__func__,__LINE__);
		printf("SERVER: got packet from %s\n", inet_ntop(server_addr.ss_family, S.getAddr((struct sockaddr *)&server_addr), destination, sizeof destination));
		printf("SERVER: packet is %d bytes long\n", bytes);
		buffer[bytes] = '\0';
		printf("SERVER Data packet contains \"%s\"\n", buffer);
		
		/* Read request */
		S.logger("Server: Read request",'d',__func__,__LINE__);
		if(buffer[0] == '0' && buffer[1] == '1')
		{
			log=S.readRequest(sockfd, buffer, server_addr, addr_len, result);
			S.logMessage("Server received READ REQUEST(RRQ) from %s \n",inet_ntop(their_addr.ss_family, S.getAddr((struct sockaddr *)&server_addr), destination, sizeof destination));
			if(log==EXIT_FAILURE)
			{
				S.logger("Server: Read request unsuccesful",'w',__func__,__LINE__);
				fprintf(stderr,"READ REQUEST UNSUCCESSFUL\n");
			}	
		}
		//Write request 
		else if (buffer[0] == '0' && buf[1] == '2')
		{
			S.logger("---Server= Write request---",'d',__func__,__LINE__);
			log=S.Upload_File(sockfd, buf, server_addr, addr_len);
			S.logmsg("Server received WRITE REQUEST(WRQ) from %s \n",inet_ntop(their_addr.ss_family, S.getAddr((struct sockaddr *)&their_addr), destination, sizeof destination));
			if(log==EXIT_FAILURE)
			{
				S.logger("---Server: Write request unsuccesful---",'w',__func__,__LINE__);
				fprintf(stderr,"WRITE REQUEST UNSUCCESSFUL\n");
			}
		} 
		else 
		{
			fprintf(stderr," ---REQUEST INVALID---\n");
			S.logmsg("Server received an invalid request from %s \n",inet_ntop(their_addr.ss_family, S.getAddr((struct sockaddr *)&server_addr), destination, sizeof destination));
			S.logger("Server: Invalid request",'f',__func__,__LINE__);
		}
	}
	// Main implementation ends 
	return EXIT_SUCCESS;
}



