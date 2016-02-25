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

#define PORT_NO 8080

void create_server();
void listen_for_request(int serv_sock);
void parse_header(char *header, int accept_sock);
void perform_action(char *path, char *method, int accept_sock);
int open_file(char *path, int accept_sock, int isOpen);
void serve_file(char *resource_path, FILE *fp, int accept_sock);
void send_header(int result, int accept_sock);

int main() {

	create_server();	

	return 0;
}

void create_server() 
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

	listen_for_request(serv_sock);

	close(serv_sock);
}

void listen_for_request(int serv_sock)
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

	bzero(buffer, 256);
	num_data_recv = read(accept_sock, buffer, 255);

	if(num_data_recv < 0)
	{
		printf("error reading from socket\n");
		exit(1);
	}

	parse_header(buffer, accept_sock);
}

void parse_header(char *header, int accept_sock) {
	char method[256];
	char path[256];

	int i = 0;
	int j = 0;
	int k = 0;

	printf("response header:\n%s", header);

	//get the method
	while(!isspace(header[i]))
	{
		method[j] = header[i];
		i++; j++;
	}

	method[j] = '\0';
	printf("%s\n", method);

	do {
		i++;
		
		if(header[i] != '/')
		{
			path[k] = header[i];			
			k++;
		}
		
	}	while(!isspace(header[i]));
	path[k-1] = '\0';

	printf("%s\n", path);

	perform_action(path, method, accept_sock);
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
			break;
		case -2: //append .html to file
			strcpy(resource_path, "resources/\0");
			strcat(resource_path, path);
			strcat(resource_path, ".html");
			fp = fopen(resource_path, "r");
			if(fp == NULL) {
				return open_file(resource_path, accept_sock, 1);
			}
			break;
		case 1: 
			printf("all attempts to open file have been exhausted...\n");
			printf("sending 404\n");
			send_header(-1, accept_sock);
			return 5;
			break;
		case 0: //file is opened
			printf("%s\n", path);
			fp = fopen(path, "r");
			serve_file(path, fp, accept_sock);
			return 5;
			break;
		default:
			return 5;
			break;
	}
	return 5;
}
void send_header(int result, int accept_sock) {
	int writeSuccess;
	char OK_response[256];
	char BAD_response[256];
	strcpy(OK_response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
	if (result == 0)
	{
		//send 200
		printf("test\n");
		writeSuccess = write(accept_sock, OK_response, strlen(OK_response));
		if(writeSuccess < 0)
		{
			printf("error writing to client\n");
			exit(1);
		}
	} else {
		//send 404
		printf("test\n");
		strcpy(BAD_response, "HTTP/1.0 404 NOT FOUND\r\n");
		strcat(BAD_response, "Content-Type: text/html\r\n\r\n");
		strcat(BAD_response, "<HTML><TITLE>404 Not Found</TITLE><BODY><H1>The server could not fufill your request...</H1></BODY></HTML>\r\n");
		writeSuccess = write(accept_sock, BAD_response, strlen(BAD_response));
	}
}
void serve_file(char *resource_path, FILE *fp, int accept_sock)
{
	struct stat st;
	char *buf;
	int writeSuccess;
	stat(resource_path, &st);

	buf = (char *)malloc((int)st.st_size);
	printf("%s\n", resource_path);
	do {
		fgets(buf, sizeof(buf), fp);
		writeSuccess = write(accept_sock, buf, strlen(buf));
	} while(!feof(fp));

}