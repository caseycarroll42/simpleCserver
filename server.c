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
	char method[500];
	char path[256];
};

struct HTTP_response {
	char buffer[100000];
	char path[256];
};

int create_server();
int connect_to_client(int serv_sock);
char * recv_request(int accept_sock);
struct HTTP_request parse_header(char *header, int accept_sock);

FILE * file_exists(char *file_path);

void perform_action(char *path, char *method, int accept_sock);
int open_file(char *path, int accept_sock, int isOpen);
int serve_file(FILE * fpRead, int accept_sock);
void send_header(int result, int accept_sock);
int connect_to_webserver(struct HTTP_request request);
int listen_webserver(struct HTTP_response response, int web_sock);
char * save_to_cache(char *url, char *buffer);




int main() {
	int serv_sock, accept_sock, web_sock;
	char header[1000];
	struct HTTP_request client_request;
	FILE * fpRead;

	
	//create socket server
	serv_sock = create_server();

	//accept connection from the client
	accept_sock = connect_to_client(serv_sock);

	//get the HTTP request from client
	strcpy(header, recv_request(accept_sock));

	//parse the HTTP request and put into struct
	client_request = parse_header(header, accept_sock);	

	if(strcasecmp(client_request.method, "GET") == 0)
	{
		//make a get request
		printf("get get get get get got got got got\n");
		if(client_request.path[0] == '\0')
		{
			printf("user requested base url... redirecting to index.html\n");
			strcat(client_request.path, "index.html");
		}
		fpRead = file_exists(client_request.path);
		if(fpRead != NULL)
		{
			//serve the file
			send_header(0, accept_sock); //send 200 OK
			serve_file(fpRead, accept_sock); //send file
		} else {
			//contact webserver
			web_sock = connect_to_webserver(client_request);
			if(web_sock < 0)
			{
				printf("error connecting to webserver\n");
			}

			struct HTTP_response response;
			listen_webserver(response, web_sock);

		}

	} else {
		printf("the request: %s is not supported\n", client_request.method);		
	}

	fclose(fpRead);
	close(serv_sock);
	close(accept_sock);

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

FILE * file_exists(char *file_path)
{
	FILE *fpRead;
	char resource_path[256];

	//create path to resources directory
	strcpy(resource_path, "resources/");
	strcat(resource_path, file_path);

	fpRead = fopen(resource_path, "r");
	if(fpRead == NULL)
	{
		fclose(fpRead);
		//append .html to resource path
		strcat(resource_path, ".html");
		fpRead = fopen(resource_path, "r");
	}

	return fpRead;

}

int connect_to_webserver(struct HTTP_request request)
{
	int sockfd, isWritten, isRead;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	printf("looking up %s\n", request.path);
	printf("this may take some time...\n");
	
	server = gethostbyname(request.path);

	if(server == NULL)
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

	sprintf(request.method, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", request.path);
	isWritten = write(sockfd, request.method, strlen(request.method));
	if(isWritten < 0)
	{
		printf("error writing to socket...\n");
		return -1;
	}

	return sockfd;
}

int listen_webserver(struct HTTP_response response, int web_sock)
{
	int bytes_read;

	bytes_read = read(web_sock, response.buffer, (sizeof(response.buffer) - 1));
	if(bytes_read < 0)
	{
		printf("error reading from web socket\n");
		return -1;
	}

	return web_sock;
}


void send_header(int result, int accept_sock) {
	int writeSuccess;
	char OK_response[256];
	char BAD_response[256];
	strcpy(OK_response, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
	if (result == 0)
	{
		//send 200
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
int serve_file(FILE *fpRead, int accept_sock)
{
	char *buf;
	int writeSuccess;
	long file_size;
	

	fseek (fpRead, 0, SEEK_END);
	file_size = ftell(fpRead);
	fseek (fpRead, 0, SEEK_SET);

	buf = (char *)malloc((int)file_size);
	//printf("%s\n", resource_path);
	do {
		fgets(buf, sizeof(buf), fpRead);
		writeSuccess = write(accept_sock, buf, strlen(buf));
	} while(!feof(fpRead));
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