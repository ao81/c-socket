#include "mysocket.h"

void trim(char* s) {
	char* p = s;
	int len = strlen(s);

	/* 末尾のスペースを削除 */
	while (len > 0 && isspace(s[len - 1])) {
		s[len - 1] = '\0';
		len--;
	}

	/* 先頭のスペースを削除 */
	while (*p && isspace(*p)) {
		p++;
	}

	/* 先頭を詰める */
	if (p != s) {
		memmove(s, p, len - (p - s) + 1);
	}
}

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
		int input;

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

		switch (polling.connType) {
		case NEW_CONN:
			printf("サーバーに接続されました！\n");
			break;

		case WAIT_CONN:
			printf("待機中です...\n");
			break;

		case NAME:
			while (true) {
				printf("名前を入力してください: ");
				if (fgets(polling.name, BUF_SIZE, stdin) == NULL) {
					continue;
				}
				int len = strlen(polling.name);

				/* 改行文字を削除 */
				if (len > 0 && polling.name[len - 1] == '\n') {
					polling.name[--len] = '\0';
				}

				/* 前後のスペースを削除 */
				trim(polling.name);

				/* 1文字以上入力されているか */
				if (strlen(polling.name) > 0) {
					break;
				}

			}
			send(sock, &polling, sizeof(polling), 0);
			break;

		case START:
			printf("ゲームを開始します！\n");
			break;

		case ACTIONS:
			printf("\nあなたは %s です。\n", polling.order ? "密輸者" : "検査官");

			switch (polling.action.type) {
			case WAIT:
				printf("待機中です...\n");
				break;

			case TRUNK:
				printf("トランクに何円入れますか？\n");
				while (true) {
					printf(">> ");
					scanf("%ld", &polling.action.trunk_amount);
					if (0 <= polling.action.trunk_amount && polling.action.trunk_amount <= MAX_TRUNK) {
						break;
					}
					printf("0以上%d以下の値を入力してください。\n", MAX_TRUNK);
				}

				polling.connType = ACTIONS;
				polling.action.type = TRUNK;

				if (send(sock, &polling, sizeof(polling), 0) == -1) {
					perror("Error: send");
					return -1;
				}
				printf("サーバーに送信しました！\n");
				break;

			case CHECK:
				printf("搭乗客が来ました！\n");
				printf("トランクに現金があるかを推理してください。\n");
				printf("ない:0 ある:1\n");
				do {
					printf(">> ");
					scanf("%d", &input);
				} while (input != 0 && input != 1);

				polling.connType = ACTIONS;

				if (input == 0) {
					polling.action.type = PASS;
				} else if (input == 1) {
					polling.action.type = DOUBT;

					// 密輸額の予想
					printf("密輸額の予想\n");
					do {
						clear_stdin();
						printf(">> ");
						scanf("%ld", &polling.action.doubt_amount);
					} while (!(1 <= polling.action.doubt_amount && polling.action.doubt_amount <= MAX_TRUNK));
				}

				if (send(sock, &polling, sizeof(polling), 0) == -1) {
					perror("Error: send");
					return -1;
				}
				printf("サーバーに送信しました！\n");
				break;

			default:
				break;
			}
			break;

		default:
			printf("不明なメッセージを受信しました: %d\n", polling.connType);
			break;
		}
	}

end:
	close(sock);
	return 0;
}