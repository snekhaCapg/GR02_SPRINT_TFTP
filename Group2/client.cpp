#include "client.h"

void User :: Blkno_to_String(char *temp, int n)
{
	if(n==0)
	{
		temp[0] = '0', temp[1] = '0', temp[2] = '\0';
	}
	else if(n%10 > 0 && n/10 == 0)
	{

		char ch = n+'0';
		temp[0] = '0', temp[1] = ch, temp[2] = '\0';
	} 
	else if(n%100 > 0 && n/100 == 0)
	{
		char second = (n%10)+'0';
		char first = (n/10)+'0';
		temp[0] = first, temp[1] = second, temp[2] = '\0';
	}
	else 
	{
		temp[0] = '9', temp[1] = '9', temp[2] = '\0';
	}
}

char* User :: ReadRequest(char *f_name)
{
	char *datapacket;
	datapacket = (char*)malloc(2+strlen(f_name)+1);
	size_t len = sizeof(datapacket);
	memset(datapacket,0,len);
	strcat(datapacket,READ_OPCODE);
	strcat(datapacket,f_name);
	return datapacket;
}


char* User :: WriteRequest(char *f_name)
{
	char *datapacket;
	datapacket = (char*)malloc(2+strlen(f_name)+1);
	size_t len = sizeof(datapacket);
	memset(datapacket,0,len);
	strcat(datapacket, WRITE_OPCODE);
	strcat(datapacket, f_name);
	return datapacket;

}

char* User :: DataPacket(int blk, char *data)
{
	char *datapacket;
	char temp[3];
	Blkno_to_String(temp,blk);
	datapacket = (char*)malloc(4+strlen(data)+1);
	size_t len = sizeof(datapacket);
	memset(datapacket,0,len);
	strcat(datapacket,DATA_OPCODE);
	strcat(datapacket,temp);
	strcat(datapacket,data);
	return datapacket;
}

char* User :: Acknowledgment(char *blk)
{
	char *datapacket = NULL;
	datapacket = (char *)malloc(2+strlen(blk)+1);
	strcpy(datapacket, ACK_OPCODE);
	strcat(datapacket, blk);
	return datapacket;
}

char* User :: Error(char *err_code,char* error_msg)
{
	char *datapacket;
	datapacket = (char*)malloc(4+strlen(error_msg)+1);
	size_t len=sizeof(datapacket);
	memset(datapacket, 0, len);
	strcat(datapacket, ERROR_OPCODE); // opcode 
	strcat(datapacket, err_code);
	strcat(datapacket, error_msg);
	return datapacket;
}


void *User :: getAddr(struct sockaddr *a)
{
	if (a->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)a)->sin_addr);
	}
	return &(((struct sockaddr_in6*)a)->sin6_addr);
	

}

int User :: Timeout(int sockfd, char *buf,struct sockaddr_store *client_addr,socklen_t len,struct info *result,char *end_msg)
{
	fd_set fds;
	int ret;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	tv.tv_sec = TIME_OUT;
	tv.tv_usec = 0;

	ret = select(sockfd+1, &fds, NULL, NULL, &tv);
	if (ret == 0)
	{
		printf("timeout\n");
		logger("Client: Timeout",'w',__func__,__LINE__);
		return -2; // timeout 
	}
	else if (ret == -1)
	{
		printf("Select error\n");
		return -1; /* error */
	}
	return recvfrom(sockfd, buf, MAXLEN-1 , 0, (struct sockaddr *)client_addr, &len);
}


int User :: Tries(int sockfd,char *buf,struct sockaddr_store *client_addr,socklen_t len,struct info *result,char *end_msg)
{
	logger("Maximum Tries Function",'d',__func__,__LINE__);
	int t;
	int bytes;
	for(t=0;t<TRY_NUM;++t)
	{
		if(t == TRY_NUM)
		{
			logger("Client: Maximum number of tries reached",'w',__func__,__LINE__);
			cout<<"Client: Maximum number of tries reached"<<endl;
			exit(1);
		}
		logger("Client: Timeout check",'d',__func__,__LINE__);
		bytes = Timeout(sockfd,buf,client_addr,len);

		if(bytes == -1)
		{
			logger("Client: select() ",'f',__func__,__LINE__);
			errorhandling(bytes,"CLIENT: recvfrom");
		}
		else if(bytes == -2)
		{
			logger("Client: timeout",'w',__func__,__LINE__);
			printf("CLIENT: try no. %d\n", t+1);
			int temp_bytes;
			if((temp_bytes = sendto(sockfd, end_msg, strlen(end_msg), 0, result->ai_addr, result->ai_addrlen)) == -1)
			{
				logger("Client: sendto",'f',__func__,__LINE__);
				errorhandling(bytes,"CLIENT ACK: sendto");
			}
			cout<<"Client: sent "<<temp_bytes<<"bytes again"<<endl;
			logger("Client sent bytes again",'i',__func__,__LINE__);
			continue;
		}
		else
		{
			break;
		}
	}
	return bytes;
}


//Reading a file
//
int User :: Download_File(int sockfd, struct sockaddr_store client_addr,struct info *result,char *file,char *server)
{
	char *msg = ReadRequest(file);
	int bytes;
	FILE *fp;
	char lst_msg[MAXLEN];
	char filename[100];
	char buf[MAXLEN];

	char *blk;
	char *trans_msg;

	strcpy(lst_msg,"");
	char ack[10];
	char destination[INET6_ADDRSTRLEN];
	socklen_t len;
	len = sizeof(client_addr);
	strcpy(ack,"");

	if((bytes = sendto(sockfd, msg, strlen(msg),0,result->ai_addr,result->ai_addrlen)) == -1)
	{
		logger("Client: sendto",'f',__func__,__LINE__);
		errorhandling(bytes,"CLient: sendto");
	}
	cout<<"Send "<<bytes<<" bytes to "<<server;
	logger("Client sent to server",'i',__func__,__LINE__);

	int flag=1;
	logger("Receiving actual file",'d',__func__,__LINE__);


	int client_msg;

	do
	{
		logger("Receiving file data packet",'i',__func__,__LINE__);
		if(bytes = recvfrom(sockfd,buf,MAXLEN-1,0,(struct sockaddr*)&client_addr, &len)==-1)
		{
			logger("Client: recvfrom",'f',__func__,__LINE__);
			errorhandling(bytes,"Client::recvfrom");
		}
		printf("Client: received packet from %s\n", inet_ntop(client_addr.ss_family, getAddr((struct sockaddr *)&client_addr),destination, INET6_ADDRSTRLEN));
		logger("Client: received packets from",'i',__func__,__LINE__);
		printf("Client: packet is %d bytes long\n", bytes);

		buf[bytes] = '\0';
		printf("Client: packet contains \"%s\"\n", buf);

		if(buf[0]=='0' && buf[1] == '5')
		{
			fprintf(stderr, "Received error packets: %s\n", buf);
			logger("Received error packets",'w',__func__,__LINE__);
			free(msg);
			exit(1);
		}

		if(flag == 1)
		{
			strcpy(filename, file);
			strcat(filename, "Client");
			fp = fopen(filename, "wb");
			if(fp == NULL)
			{
				logger("Error while openning the file",'f',__func__,__LINE__);
				fprintf(stderr,"Error while openning file: %s\n", filename);
				free(msg);
				exit(1);
			}
			flag=0;
		}

		if(strcmp(buf,lst_msg) == 0)
		{
			logger("Last acknowledgment has not reached",'w',__func__,__LINE__);
			bytes = sendto(sockfd, ack,strlen(ack),0,(struct sockaddr *)&client_addr,len);
			if(bytes == -1)
			{
				logger("Error while sending the system call",'f',__func__,__LINE__);
				errorhandling(bytes,"sendto");
			}
			continue;
		}


		client_msg = strlen(buf+4);
		fwrite(buf+4,sizeof(char),client_msg,fp);
		strcpy(lst_msg,buf);

		logger("Sending ack data packet",'d',__func__,__LINE__);
		blk = (char*)malloc(3*sizeof(char));
		strncpy(blk, buf+1,2);
		blk[2] = '\0';

		trans_msg = Acknowledgment(blk);

		if((bytes = sendto(sockfd, trans_msg, strlen(trans_msg),0,result->ai_addr,result->ai_len))==-1)
		{
			logger("Failed to send ack",'f',__func__,__LINE__);
			errorhandling(bytes,"sendto:");
		}
		printf("CLIENT: sent %d bytes\n", bytes);
		logger("Client sent data",'i',__func__,__LINE__);
		strcpy(lst_msg, trans_msg);
		strcpy(buf,lst_msg);
		free(blk);
		free(trans_msg);
	} 
	while(client_msg == MAXLEN);
		printf("NEW FILE: %s SUCCESSFULLY MADE\n", filename);
		logger("New file successfully made",'i',__func__,__LINE__);
		free(msg);
		fclose(fp);
		return EXIT_SUCCESS;
}



int User :: Upload_File(int sockfd, struct sockaddr_store client_addr,struct info *result,char *file,char *server)
{
	char *msg = WriteRequest(file);
	char *lst_msg;
	char destination[INET6_ADDRSTRLEN];
	char buf[MAXLEN];
	char *trans_msg;
	int bytes;
	if((bytes = sento(sockfd, msg, strlen(msg),0,result->ai_addr,result->ai_len))==-1)
	{
		logger("Client: sento",'f',__func__,__LINE__);
		errorhandling(bytes,"sento");
	}
	printf("Sent %d bytes to %s\n", bytes, server);
	logger("Send data to the server",'i',__func__,__LINE__);
	lst_msg = msg;

	socklen_t len;

	len = sizeof(client_addr);
	logger("maximum tries function call",'d',__func__,__LINE__);

	bytes = Tries(sockfd,buf,&client_addr,len,result,end_msg);
	printf("CLIENT: got packet from %s\n", inet_ntop(client_addr.ss_family, getAddr((struct sockaddr *)&client_addr), destination, INET6_ADDRSTRLEN));
	printf("CLIENT: packet is %d bytes long\n", bytes);

	buf[bytes] = '\0';
	printf("Packet Contains \"%s\"\n", buf);

	if(buf[0] == '0' && buf[1] == '4')
	{
		FILE *fp = fopen(file, "rb");
		if(fp == NULL || access(file, F_OK) == -1)
		{
			fprintf(stderr,"CLIENT: file %s does not exist\n", file);
			char *e_msg = makeERR("01", "ERROR_FILE_NOT_FOUND");
			printf("%s\n", e_msg);
			sendto(sockfd, e_msg, strlen(e_msg), 0, result->ai_addr, result->ai_len);
			free(e_msg);
			free(msg);
			logger("Client: File does not exist",'w',__func__,__LINE__);
			exit(1);
		}
		logger("Calculating the size of file",'d',__func__,__LINE__);
		int blk = 1;
		fseek(fp, 0, SEEK_END);
		int total = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int remaining = total;

		if(remaining == 0)
			++remaining;
		else if(remaining%READ_LEN == 0)
			--remaining;

		logger("Reading file data packet",'i',__func__,__LINE__);

		while(remaining>0)
		{
			char temp[520];
			if(remaining > READ_LEN)
			{
				fread(temp, READ_LEN, sizeof(char), fp);
				temp[READ_LEN] = '\0';
				remaining -= (READ_LEN);
			}
			else
			{
				fread(temp,remaining,sizeof(char),fp);
				temp[remaining] = '\0';
				remaining =0;
			}

			logger("Sending file data packets",'d',__func__,__LINE__);
			trans_msg = DataPacket(blk,temp);

			if((bytes = sendto(sockfd, trans_msg,strlen(trans_msg),0,result->ai_addr,result->ai_addrlen)) == -1)
			{
				logger("CLient: sendto",'f',__func__,__LINE__);
				errorhandling(bytes,"Client: snedto");
			}

			printf("CLIENT: sent %d bytes to %s\n", bytes, server);
			logger("Client: sent bytes to server",'i',__func__,__LINE__);
			lst_msg = trans_msg;

			bytes=Tries(sockfd,buf, &client_addr,len,result,end_msg);
			printf("CLIENT: got packet from %s\n", inet_ntop(client_addr.ss_family, getAddr((struct sockaddr *)&client_addr), destination, INET6_ADDRSTRLEN)); 
			logger("Client got packet",'i',__func__,__LINE__);
			printf("CLIENT: packet is %d bytes long\n", bytes);
			buf[bytes] = '\0';
			printf("CLIENT: packet contains \"%s\"\n", buf);

			if(buf[0]=='0' && buf[1]=='5')	
			{
				logger("Client: error packet received",'w',__func__,__LINE__);
				fprintf(stderr, "error while recving packet: %s",buf);
				free(trans_msg);
				free(msg);
				fclose(fp);
				exit(1);
			}
			++blk;
			if(blk>100)
				blk = 1;
			free(trans_msg);
		}
		free(msg);
		fclose(fp);
	}

	else
	{
		logger("Client: ack expecting but got",'w',__func__,__LINE__);
		fprintf(stderr,"CLIENT ACK: expecting but got: %s\n", buf);
		exit(1);
	}
	return EXIT_SUCCESS;
}




int User :: logger(char* msg, char type, const char *func, int line)
{
		char PATH[]="../log";
		time_t ltime = time(NULL);
		struct tm result;
		char TIMESTAMP[32];
		localtime_r(&ltime,&result);
		asctime_r(&result,TIMESTAMP);

		FILE *log;
		char filename[100];

		switch(type)
		{
			case 'i':
				sprintf(filename,"%s/info.log",PATH);
				log = fopen(filename,"a+");
				fprintf(log,"\n~~%s[%s : %s : %d]\t%s\n--------\n",TIMESTAMP,__FILE__,func,line,msg);
				fclose(log);
				break;

			case 'f':
				sprintf(filename,"%s/fatal.log",PATH);
                                log = fopen(filename,"a+");
				fprintf(log,"\n~~%s[%s : %s : %d]\t%s\n--------\n",TIMESTAMP,__FILE__,func,line,msg);
				fclose(log);
				break;

			case 'w':
				sprintf(filename,"%s/warning.log",PATH);
				log = fopen(filename,"a+");
				fprintf(log,"\n~~%s[%s : %s : %d]\t%s\n--------\n",TIMESTAMP,__FILE__,func,line,msg);
				fclose(log);
				break;


			case 'd':
				sprintf(filename,"%s/debug.log",PATH);
				log = fopen(filename,"a+");
				fprintf(log,"\n~~%s[%s : %s : %d]\t%s\n--------\n",TIMESTAMP,__FILE__,func,line,msg);
				fclose(log);
				break;
		}
		return EXIT_SUCCESS;
}


void User :: errorhandling(int ret, const char *msg)
{
	if(ret == -1)
	{
		perror("message");
		exit(EXIT_FAILURE);
	}
}
