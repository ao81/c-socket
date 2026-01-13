#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE 1024

void game(int c1_sock, int c2_sock) {
	int recv_size; // -1:error  0:end
	int send_size;
	int recv_buf;
	int send_buf;
	int number;
	int select;

	// client2 が接続したことを client1 に伝える
	send_buf = 1;
	send(c1_sock, &send_buf, sizeof(send_buf), 0);

	while (1) {
		printf("\nゲームを開始します。\n");

		printf("client1 の数字選択待ちです...\n");
		recv_size = recv(c1_sock, &recv_buf, sizeof(recv_buf), 0);
		if (recv_size == -1) {
			printf("Error: recv()\n");
			break;
		}
		if (recv_size == 0) {
			printf("Connection ended\n");
			break;
		}

		// client1 が入力したことを client2 に伝える
		send_buf = 1;
		send(c2_sock, &send_buf, sizeof(send_buf), 0);

		printf("client1 が入力した数字は >> %d << です。\n\n", recv_buf);
		number = recv_buf;

		printf("client2 が予想しています...\n");
		while (1) {
			recv_size = recv(c2_sock, &recv_buf, sizeof(recv_buf), 0);
			if (recv_size == -1) {
				printf("Error: recv()\n");
				break;
			}
			if (recv_size == 0) {
				printf("Connection ended\n");
				break;
			}

			printf("予想した数字は >> %d << です。\n", recv_buf);

			// 正誤判定をc2へ送信
			if (number == recv_buf) {
				send_buf = 1;
			} else {
				send_buf = -1;
			}
			send(c2_sock, &send_buf, sizeof(send_buf), 0);
			if (send_buf == 1) break;
		}

		printf("client2 が予想に正解しました!!\n");
		send_buf = 1;
		send(c1_sock, &send_buf, sizeof(send_buf), 0);

		printf("再びプレイしますか?(1:replay 0:exit) >> ");
		scanf("%d", &select);
		if (select == 1) {
			send_buf = 1;
			send(c1_sock, &send_buf, sizeof(send_buf), 0);
			send(c2_sock, &send_buf, sizeof(send_buf), 0);
		} else {
			send_buf = 0;
			send(c1_sock, &send_buf, sizeof(send_buf), 0);
			send(c2_sock, &send_buf, sizeof(send_buf), 0);
			break;
		}
	}
}

int main(void) {
	int addr;
	int c1_sock;
	int c2_sock;
	struct sockaddr_in a_addr;

	// ソケット作成
	addr = socket(AF_INET, SOCK_STREAM, 0);
	if (addr == -1) {
		printf("Error: socket()\n");
		return -1;
	}

	// 構造体を全て0にセット
	bzero(&a_addr, sizeof(struct sockaddr_in));

	// サーバーのIPアドレスとポートの情報を設定
	a_addr.sin_family = AF_INET;
	a_addr.sin_port = htons((unsigned short)SERVER_PORT);
	a_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	// TIME_WAIT状態のポートを即座に利用できるようにする
	int opt = 1;
	if (setsockopt(addr, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == -1) {
		printf("Error: setsockopt()\n");
		return -1;
	}

	// ソケットに情報を設定
	if (bind(addr, (const struct sockaddr *)&a_addr, sizeof(a_addr)) == -1) {
		printf("Error: bind()\n");
		return -1;
	}

	// ソケットを接続待ちに設定
	if (listen(addr, 3) == -1) {
		printf("Error: listen()\n");
		return -1;
	}

	//while (1) {
	printf("Waiting for client1 to connect...\n");
	c1_sock = accept(addr, NULL, NULL);
	if (c1_sock == -1) {
		printf("Error: accept()\n");
		close(addr);
		return -1;
	}
	printf("Connected client1 !!\n");

	printf("Waiting for client2 to connect...\n");
	c2_sock = accept(addr, NULL, NULL);
	if (c2_sock == -1) {
		printf("Error: accept()\n");
		close(addr);
		return -1;
	}
	printf("Connected client2 !!\n");

	game(c1_sock, c2_sock);

	close(c1_sock);
	close(c2_sock);
	//}

	close(addr);
	return 0;
}
