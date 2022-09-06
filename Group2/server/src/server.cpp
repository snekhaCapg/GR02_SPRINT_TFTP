#include <server.h>
static FILE *logFp;
void Server :: Blkno_to_String(char *temp, int n)
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
		char first = (n%10)+'0';
		char second = (n/10)+'0';
		temp[0] = first, temp[1] = second, temp[2] = '\0';
	}
	else 
	{
		temp[0] = '9', temp[1] = '9', temp[2] = '\0';
	}
}
char* Server :: DataPacket(int blk, char *data){
	char *datapacket=NULL;
	char temp[3];
	Blkno_to_String(temp, blk);
	datapacket = (char*)malloc(4+strlen(data)+1);
	strcpy(datapacket, DATA_OPCODE);
	strcat(datapacket, temp);
	strcat(datapacket, data);
	return datapacket;
}

char* Server :: Acknowledgment(char* blk){
	char *datapacket=NULL;
	datapacket = (char*)malloc(2+strlen(blk)+1);
	strcpy(datapacket, ACK_OPCODE);
	strcat(datapacket, blk); 
	return datapacket;
}
char* Server :: Error(char *err_code, char* error_msg){
	char *datapacket=NULL;
	datapacket = (char*)malloc(4+strlen(error_msg)+1);
	size_t len=sizeof(datapacket);
	memset(datapacket, 0, len);
	strcat(datapacket, ERROR_OPCODE);
	strcat(datapacket, err_code);
	strcat(datapacket, error_msg);
	return datapacket;
} 
void Server::logmsg(const char *format,...)
{
	va_list argList;
	const char *TIMEFORMAT = "%d-%m-%Y %X";	
#define DATESIZE sizeof("DD-MM-YYYY HH:MM:SS")
	char timeStamp[DATESIZE];
	time_t t;
	struct tm *tmVar;
	t = time(NULL);
	tmVar = localtime(&t);
	if(tmVar == NULL || strftime(timeStamp,DATESIZE,TIMEFORMAT,tmVar) == 0)
	{
		fprintf(logFp, "***Unknown time***: ");
	}
	else
	{
		fprintf(logFp, "%s: ",timeStamp);
	}
	va_start(argList, format);
	vfprintf(logFp, format, argList);
	fprintf(logFp,"\n");	
	va_end(argList);
}
void Server :: Openlog(const char *logfile)
{
	mode_t oldPerms;
	oldPerms = umask(077);
	logFp = fopen(logfile, "a");
	umask(oldPerms);
	if(logFp == NULL)
	{
		perror("logfile open");
		exit(EXIT_FAILURE);
	}
	setbuf(logFp, NULL);
	logmsg("opened log file");

}
void Server :: Closelog(void)
{
	logmsg("closing log file");
	fclose(logFp);
}
void* Server :: getAddr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int Server :: Timeout(int sockfd, char *buf, struct sockaddr_storage server_addr, socklen_t len){
	fd_set fds;
	int val;
	struct timeval tv;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	tv.tv_sec = TIME_OUT;
	tv.tv_usec = 0;
	val = select(sockfd+1, &fds, NULL, NULL, &tv);
	if (val == 0)
	{
		printf("timeout\n");
		//logger("Server: Timeout",'w',__func__,__LINE__);
		return -2;
	} 
	else if (val == -1)
	{
		printf("error\n");
		return -1;  
	}
	return recvfrom(sockfd, buf, MAXLEN-1 , 0, (struct sockaddr *)&server_addr, &len);
}

int Server :: Tries(int sockfd, char *buf, struct sockaddr_storage server_addr, socklen_t len, struct addrinfo *result, char *t_msg){
	int t;
	int bytes;
	for(t=0;t<=TRY_NUM;++t)
	{
		if(t==TRY_NUM)
		{
			printf("SERVER: MAX NUMBER OF TRIES REACHED\n");
			//logger("Server: Max number of tries reached",'f',__func__,__LINE__);
			return -1;
		}
		bytes = Timeout(sockfd, buf, server_addr,len);
		if(bytes == -1)
		{	
			//logger("Server: recvfrom",'f',__func__,__LINE__);
			errorhandling(bytes,"SERVER: recvfrom");
		} 
		else if(bytes == -2)
		{	
			printf("SERVER: try no. %d\n", t+1);
			int temp_bytes;
			if((temp_bytes = sendto(sockfd, t_msg, strlen(t_msg), 0, result->ai_addr, result->ai_addrlen)) == -1)
			{
				//logger("Server: sendto",'f',__func__,__LINE__);
				errorhandling(temp_bytes,"SERVER: ACK: sendto");
			}
			printf("SERVER: sent %d bytes AGAIN\n", temp_bytes);
			//logger("Server: sent bytes again",'i',__func__,__LINE__);
			continue;
		} 
		else 
		{ 
			break;
		}
	}
	return bytes;
}

int Server :: Download_File(int sockfd, char *buf, struct sockaddr_storage server_addr, socklen_t len, struct addrinfo *result)
{
	char filename[100];
	int bytes;
	char *t_msg;
	char destination[INET6_ADDRSTRLEN];
	strcpy(filename, buf+2);
	FILE *fp = fopen(filename, "rb"); 
	if(fp == NULL || access(filename, F_OK) == -1)
	{ 	
		fprintf(stderr,"SERVER: file '%s' does not exist, sending error packet\n", filename);
		char *e_msg = Error("01", "ERROR_FILE_NOT_FOUND");
		//logger("Server: Error file not found",'w',__func__,__LINE__);
		printf("%s\n", e_msg);
		//logger("Server: Sendto()",'d',__func__,__LINE__);
		sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&server_addr,len);
		free(e_msg);
		return EXIT_FAILURE;
	}
	//logger("Starting to send file",'d',__func__,__LINE__);
	int block = 1;
	fseek(fp, 0, SEEK_END);    
	int total = ftell(fp);   
	fseek(fp, 0, SEEK_SET);
	int remaining = total;
	if(remaining == 0)
		++remaining;
	else if(remaining%READ_LEN == 0)
		--remaining;
	while(remaining>0)
	{
		char temp[READ_LEN+5];
		if(remaining>READ_LEN)
		{
			fread(temp,READ_LEN, sizeof(char), fp);
			temp[READ_LEN] = '\0';
			remaining -= (READ_LEN);
		} 
		else 
		{
			fread(temp, remaining, sizeof(char), fp);
			temp[remaining] = '\0';
			remaining = 0;
		}
		t_msg = DataPacket(block, temp);
		if((bytes = sendto(sockfd, t_msg, strlen(t_msg), 0, (struct sockaddr *)&server_addr, len)) == -1)
		{
			//logger("Server: ACK sendto",'f',__func__,__LINE__);
			errorhandling(bytes,"SERVER ACK: sendto");
		}
		printf("SERVER: sent %d bytes\n", bytes);
		//logger("Server: Maxtries",'d',__func__,__LINE__);
		bytes=Tries(sockfd, buf, server_addr, len, result, t_msg);
		if(bytes==-1)
		{
			//logger("Server: failure",'w',__func__,__LINE__);
			free(t_msg);
			fclose(fp);
			return EXIT_FAILURE;
		}
		//logger("Server: Got packet from client",'i',__func__,__LINE__);
		printf("SERVER: got packet from %s\n", inet_ntop(server_addr.ss_family, getAddr((struct sockaddr *)&server_addr), destination, sizeof destination));
		printf("SERVER: packet is %d bytes long\n", bytes);
		buf[bytes] = '\0';
		printf("SERVER: packet contains \"%s\"\n", buf);				
		++block;
		if(block>100)
			block = 1;
		free(t_msg);
	}
	fclose(fp);
	return EXIT_SUCCESS;
}

int Server :: Upload_File(int sockfd, char *buf, struct sockaddr_storage server_addr, socklen_t len)
{
	char *message = Acknowledgment(WRQ_ACK);
	int bytes;
	int flag=1;
	char *t_msg=NULL;
	FILE *fp;
	char destination[INET6_ADDRSTRLEN];
	char filename[100];
	char lst_msg[MAXLEN];
	strcpy(lst_msg, buf);
	char lst_ack[10];
	strcpy(lst_ack, message);
	if((bytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, len)) == -1)
	{
		//logger("Server: ACK sendto",'f',__func__,__LINE__);
		errorhandling(bytes,"SERVER ACK: sendto");
	}
	strcpy(filename, buf+2);
	int server_msg;
	do
	{
		//logger("Server: Receiving packet data",'d',__func__,__LINE__);
		if ((bytes = recvfrom(sockfd, buf, MAXLEN-1 , 0, (struct sockaddr *)&server_addr, &len)) == -1) 
		{
			//logger("Server: recvfrom",'f',__func__,__LINE__);
			errorhandling(bytes,"SERVER: recvfrom");
		}
		//logger("Server got packet from",'i',__func__,__LINE__);
		printf("SERVER: got packet from %s\n", inet_ntop(server_addr.ss_family, getAddr((struct sockaddr *)&server_addr), destination, sizeof destination));
		printf("SERVER: packet is %d bytes long\n", bytes);
		buf[bytes] = '\0';
		printf("SERVER: packet contains \"%s\"\n", buf);
		if(flag==1)
		{
			if(buf[0]=='0' && buf[1]=='5')
			{
				fprintf(stderr, "SERVER: got error packet: %s\n", buf);
				//logger("Server: got error packet",'w',__func__,__LINE__);
				free(message);
				return EXIT_FAILURE;
			}
			if(access(filename, F_OK) != -1)
			{ 
				//logger("Server: Sending error packet",'d',__func__,__LINE__);
				fprintf(stderr,"SERVER: file %s already exists, sending error packet\n", filename);
				char *e_msg = Error("06", "ERROR_FILE_ALREADY_EXISTS");
				if((bytes=sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&server_addr, len))==-1){
					//logger("Server: sendto system call failed",'f',__func__,__LINE__);
					errorhandling(bytes,"SERVER ERROR MESSAGE: sendto");
				}
				free(e_msg);
				free(message);
				return EXIT_FAILURE;
			}
			fp = fopen(filename, "wb");
			if(fp == NULL || access(filename, W_OK) == -1)
			{ 
				//logger("Server: Access denied",'w',__func__,__LINE__);
				fprintf(stderr,"SERVER: file %s access denied, sending error packet\n", filename);
				char *e_msg = Error("02", "ERROR_ACCESS_DENIED");
				//logger("Server: sendto",'d',__func__,__LINE__);
				if((bytes=sendto(sockfd, e_msg, strlen(e_msg), 0, (struct sockaddr *)&server_addr, len))==-1)
				{
					//logger("Server: sendto system call failed",'f',__func__,__LINE__);
					errorhandling(bytes,"SERVER ERROR MESSAGE: sendto");
				}
				free(e_msg);
				free(message);
				fclose(fp);
				return EXIT_FAILURE;
			}
			flag=0;
		}
		if(strcmp(buf, lst_msg) == 0)
		{
			//logger("Server: last ack sending",'i',__func__,__LINE__);
			if((bytes=sendto(sockfd, lst_ack, strlen(lst_ack), 0, (struct sockaddr *)&server_addr, len))==-1){
				//logger("Server: sendto system call failed",'f',__func__,__LINE__);
				errorhandling(bytes,"SERVER LAST SENT ACK: sendto");
			}
			continue;
		}
		server_msg= strlen(buf+4);
		fwrite(buf+4, sizeof(char), server_msg, fp);
		strcpy(lst_msg, buf);
		char block[3];
		strncpy(block, buf+2, 2);
		block[2] = '\0';
		t_msg = Acknowledgment(block);
		if((bytes = sendto(sockfd, t_msg, strlen(t_msg), 0, (struct sockaddr *)&server_addr, len)) == -1)
		{
			//logger("Server: ack sendto",'f',__func__,__LINE__);
			errorhandling(bytes,"SERVER ACK: sendto");
		}
		//logger("Server sent to client",'i',__func__,__LINE__);
		printf("SERVER: sent %d bytes\n",bytes);
		strcpy(lst_ack, t_msg);
		free(t_msg);
	}
	while(server_msg == READ_LEN);
	//logger("Server: New file made",'i',__func__,__LINE__);
	printf("NEW FILE: %s SUCCESSFULLY MADE\n", filename);
	free(message);
	fclose(fp);
	return EXIT_SUCCESS;
}

/*int Server :: logger(char* msg, char type, const char *func, int line)
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
}*/

void Server :: errorhandling(int ret, const char *message)
{
	if(ret == -1)
	{
		perror(message);
		exit(EXIT_FAILURE);
	}
}




