#include "mysocket.h"

int main(void) {
	int sock, send_size, recv_size;
	struct sockaddr_in sv_addr;
	Polling polling;

	memset(&polling, 0, sizeof(polling));

	/* ソケット作成 */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Error: socket");
		return -1;
	}

	/* 接続先サーバー設定の初期化 */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	/* 接続開始 */
	if (connect(sock, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1) {
		perror("Error: connect");
		close(sock);
		return -1;
	}

	while (true) {
		recv_size = recv(sock, &polling, sizeof(polling), 0);
		if (recv_size == -1) {
			perror("Error: recv");
			close(sock);
			return -1;
		}
		if (recv_size == 0) {
			printf("サーバーが切断されました。\n");
			close(sock);
			return -1;
		}

		switch (polling.type) {
		case NEW_CONNECT:
			printf("正常に接続されました！\n");
			break;

		case WAIT:
			printf("待機中です...\n");
			break;

		case NAME:
			printf("名前を入力してください: ");
			if (fgets(polling.name, BUF_SIZE, stdin) == NULL) {
				perror("Error: fgets");
				return -1;
			}
			int len = strlen(polling.name);
			if (polling.name[len - 1] == '\n') {
				polling.name[--len] = '\0';
			}
			send(sock, &polling, sizeof(polling), 0);
			break;

		default:
			printf("不明なメッセージを受信しました: %d\n", polling.type);
			break;
		}
	}
}