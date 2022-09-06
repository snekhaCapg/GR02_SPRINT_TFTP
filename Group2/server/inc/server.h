#pragma once 

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>


//DEFINING MACROS
//
#define PORTNO "8080"
#define MAXLEN 560
#define READ_LEN 512
#define TRY_NUM 5

#define TIME_OUT 6
#define READ_OPCODE "01"
#define WRITE_OPCODE "02"
#define DATA_OPCODE "03"
#define ACK_OPCODE "04"
#define ERROR_OPCODE "05"
#define WRQ_ACK "00"  
using namespace std;


class Server{
	int sockfd;

	public:
		void Blkno_to_String(char *temp, int n);
		char* DataPacket(int blk, char *data);
		char* Acknowledgment(char *blk);
		char* Error(char *err_code,char* error_msg);

		void logmsg(const char *msg_format,...);
		void Openlog(const char *logfile);
		void Closelog(void);
		//int logger(char *msg, char type, const char *func, int line);

		int connection();
		void *getAddr(struct sockaddr *sa);
		int Tries(int sockfd,char *buf,struct sockaddr_storage server_addr,socklen_t len,struct addrinfo *result,char *t_msg);
		int Timeout(int sockfd, char *buf,struct sockaddr_storage server_addr,socklen_t len);
		int Download_File(int sockfd,char *buf, struct sockaddr_storage server_addr,socklen_t len,struct addrinfo *result);
		int Upload_File(int sockfd,char *buf, struct sockaddr_storage server_addr,socklen_t len);
		void errorhandling(int ret, const char *message);

	private: 

		struct addrinfo clue, *server_data, *result;
		int bytes;
		struct sockaddr_storage server_addr;
		char buffer[MAXLEN];
		socklen_t len;
		char destination[INET6_ADDRSTRLEN];
};
