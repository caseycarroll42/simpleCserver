#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 1024
#define LISTEN_PORT 8080

void parse_buffer(char *header, int client);
void serve_file(char *file_path, int client);
void send_header(int client);
void bad_path(int client);

int main()
{

	int bindSuccess, server_sock, isListening, sock_recv, bytes_recvd;
	socklen_t addr_size;
	int queueSize = 5;
	struct sockaddr_in my_addr;
	struct sockaddr_in recv_addr;
	char buf[BUF_SIZE];


	//create socket
	server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//check if socket creation successful
	if(server_sock < 0) 
	{
		printf("socket() failed\n");
		exit(0);
	}

	//make local address structure
	memset(&my_addr, 0, sizeof(my_addr)); //zero out structure
	my_addr.sin_family = AF_INET; //set address family
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //set IP
	my_addr.sin_port = htons((unsigned short)LISTEN_PORT);

	bindSuccess = bind(server_sock, (struct sockaddr *) &my_addr, sizeof (my_addr));
	if(bindSuccess < 0)
	{
		printf("bind() failed\n");
		exit(0);
	}

	isListening=listen(server_sock, queueSize);
	printf("Listening on port %d...\n", LISTEN_PORT);
	if (isListening < 0) 
	{
		printf("listen() failed\n");
		exit(0);
	}

	addr_size = sizeof(recv_addr);
	sock_recv = accept(server_sock, (struct sockaddr *) &recv_addr, &addr_size);


	bytes_recvd = recv(sock_recv, buf, BUF_SIZE, 0);
	buf[bytes_recvd]=0;		
		
	//parse the buffer
	parse_buffer(buf, sock_recv);

	close(server_sock);
	close(LISTEN_PORT);

	printf("%d closed. Server terminating...\n", LISTEN_PORT);

	return 0;
}

void parse_buffer(char *header, int client) 
{
	int i = 0, j = 0;
	char method[255];
	char url[255];
	char path[255];

	//set base path
	strcpy(path, "resources");

	//discover the method from the http request
	while(!isspace(header[i]))
	{
		method[j] = header[i];
		i++;
		j++;
	}
	method[j] = '\0'; j = 0;


	if(strcasecmp(method, "GET") == 0)
	{

		//absorb url
		i++;		
		while(!isspace(header[i]))
		{
			url[j] = header[i];
			i++; j++;
		}

		//if user requests base url, append index.html
		if(url[strlen(url) - 1] == '/') {
			strcat(url, "index.html");
			strcat(path, url);
		} else {
			//user requested specific file
			strcat(path, url);
		}
		serve_file(path, client);
	}
}

void serve_file(char *file_path, int client) {
	struct stat st;
	FILE *fp = NULL;
	char *buf;

	//open file
	fp = fopen(file_path, "r");
	
	if(fp == NULL)
	{
		//file not found
		bad_path(client);
		exit(0);
	}
	
	//send the response header to the client
	send_header(client);

	//send file contents to client
	stat(file_path, &st);//get info about file, ex: file size
	buf = (char *)malloc((int)st.st_size); //dynamically allocate buf to size of file
	//send first character from file to client
	fgets(buf, sizeof(buf), fp);

	while(!feof(fp)) {
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), fp);
	}

	printf("%s sent to client\n", file_path);
	fclose(fp);
	free(buf);
}

void bad_path(int client) {
	 char buf[1024];

	 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	 send(client, buf, strlen(buf), 0);	 
	 sprintf(buf, "Content-Type: text/html\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "your request because the resource specified\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "is unavailable or nonexistent.\r\n");
	 send(client, buf, strlen(buf), 0);
	 sprintf(buf, "</BODY></HTML>\r\n");
	 send(client, buf, strlen(buf), 0);
}

void send_header(int client) {
	char buf[1024];

	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "casey server");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}