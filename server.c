#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int main()
{

	int sock;

	sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(sock < 0) 
	{
		printf("socket() failed\n");
	}

	return 0;
}
