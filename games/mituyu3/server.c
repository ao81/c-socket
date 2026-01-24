#include "mysocket.h"

typedef enum {
	INITIAL,		/* 初期状態 */
	WAIT_CONNECT,	/* 接続待ち */
	INPUT_NAME,		/* 名前入力 */
	PLAYING_GAME,	/* ゲームプレイ */
	RESULT,			/* リザルト */
} GameType;

typedef struct {
	long money[2];		/* [0]:先行 / [1]:後攻 */
	long atm[2];		/* ATMの残高 */
	char names[2][256];	/* プレイヤー名 */
	int turnCount;		/* 現在のターン */
	int trade;			/* どちらの密輸ターンか */
	int entry[2];		/* エントリー状態（0:未エントリー / 1:済） */
	GameType type;		/* 現在の状態 */
} GameStatus;

int sendType(int clsock, ConnType type) {
	Polling poll;
	memset(&poll, 0, sizeof(poll));
	poll.connType = type;

	if (clsock > 0) {
		send(clsock, &poll, sizeof(poll), 0);
		return 0;
	}

	return -1;
}

void accept_new_players(int lsock, int cl_sock[2], GameStatus* gs, int* players) {
	int i, j;

	if (*players >= 2) return; /* 満員なら何もしない */

	int new_sock = accept(lsock, NULL, NULL);
	if (new_sock < 0) return;

	for (i = 0; i < 2; i++) {
		if (cl_sock[i] == -1) {
			cl_sock[i] = new_sock;
			gs->entry[i] = 1;
			(*players)++;
			sendType(cl_sock[i], NEW_CONNECT);
			printf("新規接続: socket %d (現在 %d 人)\n", new_sock, *players);
			break;
		}
	}

	if (*players == 2) {
		gs->type = INPUT_NAME;
		printf("2人揃ったので名前入力を要求します。\n");
		for (j = 0; j < 2; j++) {
			sendType(cl_sock[j], NAME);
		}
	}
}

void handle_client_data(int index, int cl_sock[2], GameStatus* gs, int* players) {
	Polling polling;
	int recv_size = recv(cl_sock[index], &polling, sizeof(polling), 0);
	int i;

	if (recv_size <= 0) {
		/* 切断処理 */
		(*players)--;
		printf("切断: socket %d (現在 %d 人)\n", cl_sock[index], *players);
		close(cl_sock[index]);
		cl_sock[index] = -1;
		gs->entry[index] = 0;
		gs->names[index][0] = '\0';
		gs->type = WAIT_CONNECT;
		for (i = 0; i < 2; i++) {
			if (cl_sock[i] != -1) {
				sendType(cl_sock[i], WAIT);
			}
		}
	}

	switch (gs->type) {
	case INPUT_NAME:
		if (polling.connType == NAME) {
			strncpy(gs->names[index], polling.name, sizeof(gs->names[index]) - 1);
			gs->names[index][sizeof(gs->names[index]) - 1] = '\0';
			printf("プレイヤー%dの名前を登録しました: %s\n", index + 1, polling.name);
			sendType(cl_sock[index], WAIT);
		}
		break;

	case PLAYING_GAME:
		// ゲーム処理（未実装）
		break;

	default:
		break;
	}
}

void update_game() {

}

int main(void) {
	int lsock, players = 0;
	int activity, i, j;
	int recv_size;
	int cl_sock[2] = { -1, -1 };
	struct sockaddr_in sv_addr;
	struct pollfd fds[3]; /* [0]:サーバー, [1][2]:クライアント */
	GameStatus gs;
	Polling polling;

	/* gsの初期化 */
	memset(&gs, 0, sizeof(gs));
	for (i = 0; i < 2; i++) {
		gs.atm[i] = 3600000000; /* 36億 */
	}

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

	gs.type = WAIT_CONNECT;
	printf("サーバー起動!\n");

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
			accept_new_players(lsock, cl_sock, &gs, &players);
		}

		/* クライアントからのデータ受信 */
		for (i = 0; i < 2; i++) {
			if (cl_sock[i] != -1 && (fds[i + 1].revents & POLLIN)) {
				handle_client_data(i, cl_sock, &gs, &players);
			}
		}

		if (gs.type == INPUT_NAME) {
			if (strlen(gs.names[0]) > 0 && strlen(gs.names[1]) > 0) {
				printf("全員の名前が入力されました。\n");
				printf("ゲームを開始します！\n");
				gs.type = PLAYING_GAME;
				for (j = 0; j < 2; j++) {
					sendType(cl_sock[j], START);
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