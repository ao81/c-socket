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
	int recv_size; // -1:error  0:end
	int send_size;
	int recv_buf;
	int send_buf;

	printf("client2 を待っています...\n");
	recv_size = recv(sock, &recv_buf, sizeof(recv_buf), 0);
	if (recv_size <= 0) {
		printf("サーバーとの接続が切れました。\n");
		return;
	}
	printf("client2 が接続しました!\n");

	while (1) {
		printf("数字を入力してください >> ");
		scanf("%d", &send_buf);

		send_size = send(sock, &send_buf, sizeof(send_buf), 0);
		if (send_size == -1) {
			printf("Error: send()\n");
			break;
		}

		printf("client2 の正解を待っています...\n");
		recv_size = recv(sock, &recv_buf, sizeof(recv_buf), 0);
		if (recv_size <= 0) {
			printf("サーバーとの接続が切れました。\n");
			return;
		}
		printf("client2 が正解しました!\n");

		printf("server の選択待ちです...\n");
		recv_size = recv(sock, &recv_buf, sizeof(recv_buf), 0);
		if (recv_size <= 0) {
			printf("サーバーとの接続が切れました。\n");
			break;
		}
		if (recv_buf == 1) {
			printf("プレイを続行します。\n");
		} else {
			printf("プレイを終了します。\n");
			break;
		}
	}
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
