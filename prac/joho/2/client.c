#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
int main(void)
{
	int sock_id, recv_size, recv_sock;
	int random_number;
	struct sockaddr_in addr;
	struct hostent * hp;
	char buffer[256];
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_id == -1) {
		return -1;
	}
	memset(&addr, 0x00, sizeof(addr));
	hp = gethostbyname( "127.0.0.1" );
	if( hp == NULL ) {
		printf("gethostbyname is NULL\n");
		return -1;
	}
	memcpy(hp->h_addr, (struct sockaddr *)&addr.sin_addr, hp->h_length);
	addr.sin_port = htons( 27001 ); // ポート番号は 27000 + 出席番号
	addr.sin_family = AF_INET;
	if( connect(sock_id, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		perror("socket");
		printf("connect error: %d\n", errno);
		return -1;
	}

	send(sock_id, "Hello from client", strlen("Hello from client") + 1, 0);
	// printf("send to server\n");

	recv_size = recv(sock_id, buffer, sizeof(buffer), 0);
	if(recv_size == 0 || recv_size == -1) {
		close(sock_id);
		printf("recv error\n");
		return -1;
	}
	printf("received: %s\n", buffer);

	send(sock_id, "random", strlen("random") + 1, 0);

	recv_size = recv(sock_id, &random_number, sizeof(random_number), 0);
	if(recv_size == 0 || recv_size == -1) {
		close(sock_id);
		printf("recv error\n");
		return -1;
	}
	printf("received: %d\n", random_number);

	close(sock_id);

	return 0;
}
