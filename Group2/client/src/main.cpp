#include "client.h"

int sockfd;
User user;

int main(int argc, char* argv[])
{
	struct addrinfo clue, *server_data, *result;
	struct sockaddr_storage client_addr;
	int val;
	client_addr.ss_family = AF_UNSPEC;

	if(argc != 4)
	{
		//Checks if args is valid 
		//C.logger("Invalid number of arguments",'w',__func__,__LINE__);
		fprintf(stderr,"USAGE: tftp_c GET/PUT server filename\n");
		exit(1);
	}
						
	char server[100];
	strcpy(server, argv[2]);
	char file[100];
	strcpy(file,argv[3]);	
					


	memset(&clue, 0, sizeof(clue));
	clue.ai_family = AF_UNSPEC;
	clue.ai_socktype = SOCK_DGRAM;

	if((val = getaddrinfo(server, PORTNO, &clue, &server_data))!= 0)
	{
		//user.logger("Client: getAddress_Information",'f',__func__,__LINE__);
		fprintf(stderr, "CLIENT: getAddr: %s\n", gai_strerror(val));
		return 1;
	}

	for(result = server_data; result != NULL; result = result->ai_next)
	{
		if((sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == -1)
		{
			//user.logger("CLIENT: SOCKET",'w',__func__,__LINE__);
			perror("Client: socket");
			continue;
		}
		break;
	}

	if(result == NULL)
	{
		//user.logger("Client: failed to bind",'f',__func__,__LINE__);
		fprintf(stderr,"Client: failed to bind socket\n");

		return 2;
	}
	

	if(strcmp(argv[1],"GET")==0 || strcmp(argv[1],"get")==0)
	{
		//user.logger("Client: Requesting file from server",'i',__func__,__LINE__);
		user.Download_File(sockfd,client_addr,result,file,server);
	}
	else if(strcmp(argv[1],"PUT")==0 || strcmp(argv[1],"put")==0)
	{
		//user.logger("Client: Writing file to server",'i',__func__,__LINE__);
		user.Upload_File(sockfd,client_addr,result,file,server);
	}
	else
	{
		//user.logger("Client: Invalid Input Format",'w',__func__,__LINE__);
	       	fprintf(stderr,"USAGE: tftp_c GET/PUT server filename\n");
       		exit(1);
	}

	freeaddrinfo(server_data);
	close(sockfd);
	return EXIT_SUCCESS;
}

