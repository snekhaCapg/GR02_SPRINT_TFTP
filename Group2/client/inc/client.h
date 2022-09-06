#pragma once

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<time.h>

//MACROS

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

using namespace std;


class User
{
	int sockfd;

	public:
		void Blkno_to_String(char *str, int n);
		char* ReadRequest(char *f_name);
		char* WriteRequest(char *f_name);
		char* DataPacket(int blk, char *data);
		char* Acknowledgment(char *blk);
		char* Error(char *err_code, char* error_msg);
		void *getAddr(struct sockaddr *sa);


		//int logger(char* msg, char type, const char *func, int line);

		int connection();
		int Tries(int sockfd,char *buf,struct sockaddr_storage *client_addr,socklen_t len,struct addrinfo *result,char *end_msg);
		int Timeout(int sockfd, char *buf,struct sockaddr_storage *client_addr,socklen_t len);
		int Download_File(int sockfd, struct sockaddr_storage client_addr,struct addrinfo *result,char *file,char *server);
		int Upload_File(int sockfd, struct sockaddr_storage client_addr,struct addrinfo *result,char *file,char *server);
		void errorhandling(int ret, const char *msg);
	


	private:

		struct addrinfo clue, *server_data, *result;
		struct sockaddr_storage client_addr;
};	

