#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>


#define BUF_SIZE 1024
#define LISTEN_PORT 8080

void parse_buffer(char *header, int client);
void serve_file(FILE *fp, int client, char *file_path);
void send_header(int client);
void bad_path(int client);
void request_webserver(char *url, int client);

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
	FILE *fp;

	//set base path
	strcpy(path, "resources/");
	printf("%s\n",header );

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
			if(header[i] != '/') {
				url[j] = header[i]; 
				j++;
			}
			i++;
		}

		//see if file exists in resources directory
		strcat(path, url);
		printf("%s\n",path );
		fp = fopen(path, "r");
		
		if(fp == NULL) 
		{
			printf("file doesn't exist on server\n");
			fclose(fp);

			request_webserver(url, client);
		} else {
			serve_file(fp, client, path);
		}
		
	}
}

void serve_file(FILE *fp, int client, char *file_path) {
	struct stat st;
	char *buf;
	
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

void request_webserver(char *url, int client)
{
	printf("%s\n", url);

	struct hostent* webserver_info;
	struct sockaddr_in addr_send;

	int sock_send, send_len, bytes_sent, isConnected;
	

	if((webserver_info = gethostbyname(url)) == NULL)
	{
		herror("gethostbyname");
		return;
	}

	char *ip_address = inet_ntoa(*(struct in_addr *)webserver_info->h_addr);
	
	
	//strcpy(ip_address, inet_ntoa(*addr_list[0]));
	printf("%s\n", ip_address);

	sock_send = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock_send < 0)
	{
		printf("socket failed\n");
		exit(0);
	}

	memset(&addr_send, 0, sizeof(addr_send));
	addr_send.sin_family = AF_INET;
	addr_send.sin_addr.s_addr = inet_addr(ip_address);
	addr_send.sin_port = htons(80);

	isConnected = connect(sock_send, (struct sockaddr *) &addr_send, sizeof(struct sockaddr));
	if(isConnected < 0) 
	{
		printf("connection failed\n");
		exit(0);
	}

	close(sock_send);
	
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