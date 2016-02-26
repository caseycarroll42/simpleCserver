#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define PORT_NO 8080

struct HTTP_request {
	char method[256];
	char path[256];
};

int create_server();
int connect_to_client(int serv_sock);
char * recv_request(int accept_sock);


struct HTTP_request parse_header(char *header, int accept_sock);

void perform_action(char *path, char *method, int accept_sock);
int open_file(char *path, int accept_sock, int isOpen);
int serve_file(char *resource_path, int accept_sock);
void send_header(int result, int accept_sock);
int connect_to_webserver(char *url);
char * save_to_cache(char *url, char *buffer);




int main() {
	int serv_sock, accept_sock;
	char header[256];
	struct HTTP_request client_request;
	
	//create socket server
	serv_sock = create_server();

	//accept connection from the client
	accept_sock = connect_to_client(serv_sock);

	//get the HTTP request from client
	strcpy(header, recv_request(accept_sock));

	//parse the HTTP request and put into struct
	client_request = parse_header(header, accept_sock);

	printf("%s\n%s\n",client_request.method, client_request.path);

	if(strcasecmp(client_request.method, "GET") == 0)
	{
		//make a get request
		printf("get get get get get got got got got\n");
	} else {
		printf("the request: %s is not supported\n", client_request.method);		
	}

	return 0;
}

int create_server() 
{
	int serv_sock, clilen;
	
	struct sockaddr_in serv_addr;
	

	//create socket
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);

	if(serv_sock < 0)
	{
		printf("Socket could not be opened...\n");
		exit(1);
	}

	//initialize socket structure
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_NO);

	//bind the socket to the port
	if(bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("error while binding...\n");
		exit(1);
	}

	return(serv_sock);
}

int connect_to_client(int serv_sock)
{
	struct sockaddr_in cli_addr;
	socklen_t addr_size;

	int accept_sock, num_data_recv;
	char buffer[256];

	listen(serv_sock, 5);
	printf("listening on port %d\n", PORT_NO);

	addr_size = sizeof(cli_addr);

	accept_sock = accept(serv_sock, (struct sockaddr *)&cli_addr, &addr_size);
	if(accept_sock < 0)
	{
		printf("error accepting\n");
		exit(1);
	}

	return accept_sock;
}

char * recv_request(accept_sock)
{
	char static buffer[256];
	int num_data_recv;

	bzero(buffer, 256);
	num_data_recv = read(accept_sock, buffer, 255);

	if(num_data_recv < 0)
	{
		printf("error reading from socket\n");
		exit(1);
	}

	return buffer;
}

struct HTTP_request parse_header(char *header, int accept_sock) {	

	struct HTTP_request client_request;

	//counters
	int i = 0;
	int j = 0;
	int k = 0;

	printf("response header:\n%s", header);

	//get the method
	while(!isspace(header[i]))
	{
		client_request.method[j] = header[i];
		i++; j++;
	}

	client_request.method[j] = '\0';
	printf("%s\n", client_request.method);

	do {
		i++;
		
		if(header[i] != '/')
		{
			client_request.path[k] = header[i];			
			k++;
		}
		
	}	while(!isspace(header[i]));
	client_request.path[k-1] = '\0';

	printf("%s\n", client_request.path);

	return client_request;
}

void perform_action(char *path, char *method, int accept_sock)
{
	int isOpen;
	if(strcasecmp("GET", method) == 0)
	{
		isOpen = open_file(path, accept_sock, -1);
		printf("%d\n", isOpen);

	} else if (strcasecmp("DELETE", method) == 0)
	{
		printf("action not supported!\n");
	} else if (strcasecmp("POST", method) == 0)
	{
		printf("action not supported!\n");
	} else {
		printf("error performing action: %s\n", method);
	}
}

int open_file(char *path, int accept_sock, int isOpen)
{
	FILE *fp = NULL;
	char resource_path[256];

	switch(isOpen)
	{
		case -1: //file is not opened, FIRST ATTEMPT
			strcpy(resource_path, "resources/\0");
			strcat(resource_path, path);
			fp = fopen(resource_path, "r");
			if(fp == NULL)
			{
				return open_file(path, accept_sock, -2);
			} else {
				return open_file(resource_path, accept_sock, 0); 
			}
			fclose(fp);
			break;
		case -2: //append .html to file
			strcpy(resource_path, "resources/\0");
			strcat(resource_path, path);
			strcat(resource_path, ".html");
			fp = fopen(resource_path, "r");
			if(fp == NULL) {
				return open_file(path, accept_sock, -3);
			} else {
				return open_file(resource_path, accept_sock, 0);
			}
			fclose(fp);
			break;
		case -3: //try to connect to webserver
			if(connect_to_webserver(path) < 0){
				printf("trying to connect to webserver...\n");
				return open_file(resource_path, accept_sock, 1);
			} else {
				return 5;
			}
			fclose(fp);
			break;
		case 1: 
			printf("all attempts to open file have been exhausted...\n");
			printf("sending 404\n");
			send_header(-1, accept_sock);
			return 5;
			break;
		case 0: //file is opened
			printf("serving this file: %s\n", path);
			serve_file(path, accept_sock);				
			return 5;
			break;
		default:			
			return 5;
			break;
	}
	return 5;
}
int connect_to_webserver(char *url)
{
	int sockfd, isWritten, isRead;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	size_t nbytes = 100000;
	char buffer[nbytes];
	char path[256];

	printf("looking up %s\n", url);
	printf("this may take some time...\n");
	
	server = gethostbyname(url);

	if(server ==NULL)
	{
		herror("can't connect to webserver\n");
		return -1;
	}

	char *ip_address = inet_ntoa(*(struct in_addr *)server->h_addr);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("error creating socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip_address);
	serv_addr.sin_port = htons(80);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("error while attempting to connect...\n");
		return -1;
	}

	sprintf(buffer, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", url);
	isWritten = write(sockfd, buffer, strlen(buffer));
	if(isWritten < 0)
	{
		printf("error writing to socket...\n");
		return -1;
	}

	isRead = read(sockfd, buffer, (nbytes-1));
	if(isRead < 0)
	{
		printf("error reading from socket\n");
		return -1;
	}

	strcpy(path, save_to_cache(url, buffer));
	printf("did this work %s\n", path);
	serve_file(path, sockfd);

	printf("%s\n", buffer);
	return 0;

}
void send_header(int result, int accept_sock) {
	int writeSuccess;
	char OK_response[256];
	char BAD_response[256];
	strcpy(OK_response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
	if (result == 0)
	{
		//send 200
		printf("test in OK\n");
		writeSuccess = write(accept_sock, OK_response, strlen(OK_response));
		if(writeSuccess < 0)
		{
			printf("error writing to client\n");
			exit(1);
		}
	} else {
		//send 404
		printf("test in 404\n");
		strcpy(BAD_response, "HTTP/1.0 404 NOT FOUND\r\n");
		strcat(BAD_response, "Content-Type: text/html\r\n\r\n");
		strcat(BAD_response, "<HTML><TITLE>404 Not Found</TITLE><BODY><H1>The server could not fufill your request...</H1></BODY></HTML>\r\n");
		writeSuccess = write(accept_sock, BAD_response, strlen(BAD_response));
	}
}
int serve_file(char *resource_path, int accept_sock)
{
	struct stat st;
	char *buf;
	int writeSuccess;
	FILE *fp;

	fp = fopen(resource_path, "r");
	if(fp == NULL)
	{
		printf("open failed\n");
		return -1;
	}

	stat(resource_path, &st);

	buf = (char *)malloc((int)st.st_size);
	printf("%s\n", resource_path);
	do {
		fgets(buf, sizeof(buf), fp);
		writeSuccess = write(accept_sock, buf, strlen(buf));
	} while(!feof(fp));
	return 0;
}

char * save_to_cache(char *url, char *buffer)
{
	FILE *fp;

	static char path[256];

	strcpy(path, "resources/");
	strcat(path, url);
	strcat(path, ".html");

	printf("%s\n", path);
	fp = fopen(path, "w");
	if(fp == NULL)
	{
		printf("error opening file: %s\n", path);
		return "/0";
	}
	fprintf(fp, "%s", buffer);
	fclose(fp);
	return path;
}