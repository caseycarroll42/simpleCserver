#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUF_SIZE 1024
#define LISTEN_PORT 60000

int main()
{

	int bindSuccess, server_sock, isListening, sock_recv, bytes_recvd;
	int queueSize = 5;
	struct sockaddr_in my_addr;
	struct sockaddr_in recv_addr;
	char buf[BUF_SIZE];


	//create socket
	server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //might need to change to IPPROTO_IP?

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
	}

	isListening=listen(server_sock, queueSize);
	if (isListening < 0) 
	{
		printf("listen() failed\n");
		exit(0);
	}

	addr_size = sizeof(recv_addr);
	sock_recv = accept(server_sock, (struct sockaddr *) &recv_addr, &addr_size);

	while(1) {
		bytes_recvd = recv(sock_recv, bug, BUF_SIZE, 0);
		buf[bytes_recvd]=0;
		printf("Received: %s\n", buf);
	}

	close(sock_recv);
	close(server_sock);

	return 0;
}
