#include "mysocket.h"

typedef struct {
	int money[2];		/* [0]:先行 / [1]:後攻 */
	char names[2][256];	/* プレイヤー名 */
	int turnCount;		/* 現在のターン */
	int trade;			/* どちらの密輸ターンか */
	int entry[2];		/* エントリー状態（0:未エントリー / 1:済） */
	int type;			/* 現在の状態 */
} Gametype;

int sendType(int clsock[2], Type type) {
	int i;
	Polling poll;
	memset(&poll, 0, sizeof(poll));
	poll.type = type;

	for (i = 0; i < 2; i++) {
		if (clsock[i] > 0) {
			send(clsock[i], &poll, sizeof(poll), 0);
		}
	}
}

int main(void) {
	int lsock, players = 0;
	int activity, i, j;
	int recv_size;
	int cl_sock[2] = { -1, -1 };
	struct sockaddr_in sv_addr;
	struct pollfd fds[3]; /* [0]:サーバー, [1][2]:クライアント */
	Gametype gs;
	Polling polling;

	memset(&gs, 0, sizeof(gs));

	/* ソケット作成 */
	lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (lsock == -1) {
		perror("Error: socket");
		return -1;
	}

	/* サーバーの設定 */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	/* TIME_WAIT状態を回避 */
	int opt = 1;
	if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("Error: setsockopt");
		close(lsock);
		return -1;
	}

	/* ソケットに設定を紐づけ */
	if (bind(lsock, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1) {
		perror("Error: bind");
		close(lsock);
		return -1;
	}

	/* 接続待ち状態にする */
	if (listen(lsock, 2) == -1) {
		perror("Error: listen");
		close(lsock);
		return -1;
	}

	while (true) {
		/* サーバーソケット */
		fds[0].fd = lsock;
		fds[0].events = POLLIN;

		/* クライアントソケット */
		for (i = 0; i < 2; i++) {
			fds[i + 1].fd = cl_sock[i];
			fds[i + 1].events = POLLIN;
		}

		/* 監視開始 */
		activity = poll(fds, 3, -1);
		if (activity == -1) {
			perror("Error: poll");
			break;
		}

		/* 新規接続（lsockに動きがあれば） */
		if (fds[0].revents & POLLIN) {
			int new_sock = accept(lsock, NULL, NULL);
			printf("new connection accepted.\n");

			if (new_sock < 0) {
				perror("Error: accept");
			} else {
				int added = 0;
				for (i = 0; i < 2; i++) {
					if (cl_sock[i] == -1) {
						cl_sock[i] = new_sock;
						gs.entry[i] = 1;
						players++;
						added = 1;

						printf("新規接続: socket %d (現在 %d 人)\n", new_sock, players);

						sendType(cl_sock, NEW_CONNECT);

						/* 1人目 */
						if (players == 1) {
							/*polling.type = NEW_CONNECT;
							send(cl_sock[i], &polling, sizeof(polling), 0);*/
						}

						/* 2人目 */
						if (players == 2) {
							printf("2人揃いました。名前入力を要求します。\n");
							sendType(cl_sock, NAME);
						}

						break;
					}
				}

				if (added == 0) {
					printf("満員なので切断しました。(socket %d)\n", new_sock);
					close(new_sock);
				}
			}
		}

		/* クライアントからのデータ受信 */
		for (i = 0; i < 2; i++) {
			if (cl_sock[i] != -1 && (fds[i + 1].revents & POLLIN)) {
				recv_size = recv(cl_sock[i], &polling, sizeof(polling), 0);

				if (recv_size <= 0) {
					/* 切断 */
					players--;
					printf("切断: socket %d (現在 %d 人)\n", cl_sock[i], players);
					close(cl_sock[i]);
					cl_sock[i] = -1;

					/* 1人なら待機状態に戻す */
					if (players == 1) {
						sendType(cl_sock, WAIT);
					}
				} else {
					switch (polling.type) {
					case NAME:
						strncpy(gs.names[i], polling.name, sizeof(gs.names[i]) - 1);
						gs.names[i][sizeof(gs.names[i]) - 1] = '\0';
						printf("プレイヤー%dの名前を登録しました: %s\n", i + 1, polling.name);

						if (strlen(gs.names[0]) != 0 && strlen(gs.names[1]) != 0) {
							printf("ゲームを開始します!\n");
							sendType(cl_sock, START);
							sleep(1);
							goto end; /* 未実装のため */
						}

						break;

					default:
						printf("不明なメッセージを受信しました: %d\n", polling.type);
						break;
					}
				}
			}
		}
	}

end:
	for (i = 0; i < 2; i++) {
		if (cl_sock[i] > 0) {
			close(cl_sock[i]);
		}
	}
	close(lsock);
	return 0;
}