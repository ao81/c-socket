#include "common.h"

int main(void) {
	int sock;
	int send_size, recv_size;
	GamePacket packet;
	struct sockaddr_in sv_config;

	// ソケット作成
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Error: socket");
		return -1;
	}

	// 接続先サーバー設定の初期化
	memset(&sv_config, 0, sizeof(sv_config));
	sv_config.sin_family = AF_INET;
	sv_config.sin_port = htons((unsigned short)SERVER_PORT);
	sv_config.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	// 接続開始
	printf("接続を開始します...\n");
	if (connect(sock, (struct sockaddr*)&sv_config, sizeof(struct sockaddr_in)) == -1) {
		perror("Error: connect");
		close(sock);
		return -1;
	}
	printf("接続されました！\n\n");

	int end_flag = 0;

	while (!end_flag) {
		memset(&packet, 0, sizeof(GamePacket));
		recv_size = recv(sock, &packet, sizeof(GamePacket), 0);
		if (recv_size <= 0) {
			perror("Error: recv");
			close(sock);
			return -1;
		}

		// 届いたパケットの種類（type）に応じて処理を分ける
		switch (packet.type) {
			MessageType prevType;

		case WAIT_PL1:
			printf("プレイヤー1を待っています...\n");
			prevType = WAIT_PL1;
			break;

		case WAIT_PL2:
			printf("プレイヤー2を待っています...\n");
			prevType = WAIT_PL2;
			break;

		case INPUT_NAME:
			while (1) {
				// 待ち状態だった場合プレイヤーの接続メッセージを表示
				if (prevType == WAIT_PL1) {
					printf("プレイヤー1が接続されました！\n\n");
					prevType = NONE;
				}
				if (prevType == WAIT_PL2) {
					printf("プレイヤー2が接続されました！\n\n");
					prevType = NONE;
				}

				// プレイヤー名を登録
				printf("プレイヤー名を入力してください。\n> ");
				if (fgets(packet.name, sizeof(packet.name), stdin) == NULL) {
					continue;
				}
				// 改行文字を削除
				size_t len = strlen(packet.name);
				if (len > 0 && packet.name[len - 1] == '\n') {
					packet.name[len - 1] = '\0';
				}
				// 1文字以上入力されていればOK
				if (strlen(packet.name) > 0) {
					break;
				}
			}
			// 送信
			packet.type = SEND;
			send_size = send(sock, &packet, sizeof(packet), 0);
			break;

		case GAME_START:
			printf("ゲームを開始します!\n");
			// ゲーム開始処理 --------------------------未実装
			break;

		case SMUGGLE:
		case INSPECT:
		case RESULT:

		case GAME_OVER:
			printf("ゲーム終了です。\n");
			end_flag = 1;
			break;

		default:
			printf("不明なメッセージを受信しました: %d\n", packet.type);
			break;
		}
	}

	close(sock);
	return 0;
}
