#include "mysocket.h"
int createSocket(void)

{
	int sock_id;
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_id == -1)
	{
		printf("socket not created\n");
		return -1;
	}
	return sock_id;
}

int listenServer(int sock_id, int port)
{
	struct sockaddr_in saddr;
	memset(&saddr, 0x00, sizeof(saddr));
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
	{
		return -1;
	}
	if (listen(sock_id, 1) != 0)
	{
		return -1;
	}

	return 0;
}

int connectServer(int sock_id, char *hostname, int port)
{
	struct sockaddr_in addr;
	struct hostent *hp;
	memset(&addr, 0x00, sizeof(addr));
	hp = gethostbyname(hostname);
	if (hp == NULL)
	{
		printf("gethostbyname is NULL\n");
		return -1;
	}
	memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	if (connect(sock_id, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		perror("socket");
		printf("connect error: %d\n", errno);
		return -1;
	}

	return 0;
}
