#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int main()
{

	int bindSuccess, sock, isListening;
	int queueSize = 5;
	struct my_addr;
	unsigned short listen_port=6000;

	//create socket
	sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	//check if socket creation successful
	if(sock < 0) 
	{
		printf("socket() failed\n");
	}

	//make local address structure
	memset(&my_addr, 0, sizeof(my_addr)); //zero out structure
	my_addr.sin_family = AF_INET; //set address family
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //set IP
	my_addr.sin_port = htons(listen_port);

	bindSuccess = bind(sock, (struct sockaddr *) &my_addr, sizeof (my_addr));
	if(bindSuccess < 0)
	{
		printf("bind() failed\n");
	}

	isListening=listen(sock, queueSize);
	if (isListening < 0) 
	{
		printf("listen() failed\n");
	}

	return 0;
}
