#include <Socket.h>
#include<fstream>

//void download(int, struct sockaddr*,socklen_t, char *);
int main()
{

	ifstream fs;
	ofstream ft;
	char ch;
	string fname2;

	int sockfd;
	struct sockaddr_in server_addr, client_addr;

	char server_msg[SIZE];
	char client_msg[SIZE];

	int server_addr_len = sizeof(server_addr);
	int client_addr_len = sizeof(client_addr);

	memset(server_msg,'\0',sizeof(server_msg));
	memset(client_msg,'\0',sizeof(client_msg));

	sockfd =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(sockfd < 0)
	{
		perror("socket() error");
		exit(EXIT_FAILURE);
	}

	cout<<"Client Socket is created"<<endl;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORTNO);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	
	
	//download(sockfd, (struct sockaddr*)&server_addr,sizeof(server_addr,client_msg);
	//string cmsg;
	//getline(cin,cmsg);
	//strcpy(client_msg,(const char*)cmsg.c_str());
	

	string fname1;
	cout<<"Enter file to download"<<endl;
	cin>>fname1;

	strcpy(client_msg,(const char*)fname1.c_str());
	if(sendto(sockfd, client_msg, strlen(client_msg), 0,(struct sockaddr*)&server_addr, server_addr_len)<0)
	{
		perror("sendto() error");
		exit(EXIT_FAILURE);
	}

	cout<<"Sent \""<<client_msg<<"\" to the server"<<endl;


	int retRF = recvfrom(sockfd, server_msg, sizeof(server_msg), 0,(struct sockaddr*)&server_addr, (socklen_t*)&server_addr_len);

	if(retRF < 0)
	{
		perror("recvfrom() error");
		exit(EXIT_FAILURE);
	}

	cout<<"\n\nReceived from Server:\n\n\t"<<server_msg<<endl;


	
	fs.open(fname1);
	if(!fs)
	{
		cout<<"Error...File not found"<<endl;
		exit(1);
	}
	cout<<"Enter the file name to copy";
	cin>>fname2;
	ft.open(fname2);

	while(fs.eof() == 0)
	{
		fs>>ch;
		ft<<ch;
	}
	cout<<"File downloaded Successfully...."<<endl;
	fs.close();
	ft.close();
	
		

	

	close(sockfd);

	return 0;
}


/*void download(int sockfd, struct sockaddr*, socklen_t length, char *fname)
{
	ifstream fs;
	ofstream ft;
	char ch, fname1[20], fname2[20];
	cout<<"Enter source file name in Server";
	cin>>fname1;
	fs.open(fname1);

	if(!fs)
	{
		cout<<"Error"<<endl;
		cout<<"File not found";
		exit(1);
	}
	cout<<"Enter filename to copy";
	cin>>fname2;


	if(!ft)
	{
		cout<<"Error in openning target file.. "<<endl;
		fs.close();
		exit(2);
	}
	while(fs.eof() == 0)
	{
		fs>>ch;
		ft<<ch;
	}

	cout<<"File Downloaded successfully.."<<endl;

	fs.close();
	ft.close();
	return 0;
}*/
