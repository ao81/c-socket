#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE 1024

void game(int c_sock) {

}

int main(void) {
	int w_addr;
	int c_sock;
	struct sockaddr_in a_addr;

	// ソケット作成
	w_addr = socket(AF_INET, SOCK_STREAM, 0);
	if (w_addr == -1) {
		printf("Error: socket()\n");
		return -1;
	}

	// 構造体を全て0にセット
	bzero(&a_addr, sizeof(struct sockaddr_in));

	// サーバーのIPアドレスとポートの情報を設定
	a_addr.sin_family = AF_INET;
	a_addr.sin_port = htons((unsigned short)SERVER_PORT);
	a_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	// ソケットに情報を設定
	if (bind(w_addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1) {
		printf("Error: bind()\n");
		return -1;
	}

	// ソケットを接続待ちに設定
	if (listen(w_addr, 3) == -1) {
		printf("Error: listen()\n");
		return -1;
	}

	while (1) {
		printf("Waiting for a client's connection...\n");
		c_sock = accept(w_addr, NULL, NULL);
		if (c_sock == -1) {
			printf("Error: accept()\n");
			close(w_addr);
			return -1;
		}
		printf("Connected!!\n");

		game(c_sock);

		close(c_sock);
	}

	close(w_addr);
	return 0;
}
