#include <Socket.h>

int main()
{
	int sockfd;

	struct sockaddr_in server_addr, client_addr;

	char server_msg[SIZE];
	char client_msg[SIZE];

	int client_addr_len = sizeof(client_addr);
	int server_addr_len = sizeof(server_addr);

	memset(server_msg,'\0',sizeof(server_msg));
	memset(client_msg,'\0',sizeof(client_msg));

	sockfd =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(sockfd < 0)
	{
		perror("socket() error");
		exit(EXIT_FAILURE);
	}

	cout<<"Server Socket is created"<<endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORTNO);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	int rbind = bind(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr));
	if(rbind < 0)
	{
		perror("bind() error");
		exit(EXIT_FAILURE);	
	}

	cout<<"Server done with the bind"<<endl;

	cout<<"\nServer is now open for clients to send/recv msgs"<<endl;
	while(1){
		memset(client_msg,'\0',sizeof(client_msg));
		if(recvfrom(sockfd, client_msg, sizeof(client_msg), 0,
			(struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len)<0)
		{
			perror("recvfrom() error");
			exit(EXIT_FAILURE);	
		}
	


		cout<<"Message from the client:\n"<<client_msg<<endl;
		if(!client_msg)
		{
			perror("Error in clientmsg");
			exit(EXIT_FAILURE);
		}

		
		memset(server_msg,'\0',sizeof(server_msg));
		strcpy(server_msg,"From Server: ");
		strcat(server_msg,client_msg);
		sleep(1);
		if(sendto(sockfd, server_msg, strlen(server_msg), 0,
			(struct sockaddr*)&client_addr, client_addr_len)<0)
		{
			perror("sendto() error");
			exit(EXIT_FAILURE);	
		}
	


		cout<<"Sent Message to the client:\n"<<endl;
		if(strcmp(client_msg,":bye")==0){
			cout<<"Bye"<<endl;
			break;
		}

	}
	// strcpy(server_msg,"Welcome to Server");

	
	// int retRF = sendto(sockfd, server_msg, sizeof(server_msg), 0,
	// 	(struct sockaddr*)&client_addr, client_addr_len);

	// if(retRF < 0)
	// {
	// 	perror("sendto() error");
	// 	exit(EXIT_FAILURE);
	// }

	close(sockfd);

	return 0;
}
