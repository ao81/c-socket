#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE 1024

void game(int sock) {

}

int main(void) {
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Error: socket()\n");
		return -1;
	}

	bzero(&addr, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons((unsigned short)SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	printf("Start connect...\n");
	if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		printf("Error: connect()\n");
		close(sock);
		return -1;
	}
	printf("Connected!!\n");

	game(sock);

	close(sock);
	return -1;
}
