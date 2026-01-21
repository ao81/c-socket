#include "common.h"

int main(void) {
	int sv_sock, cl_sock[2];
	int send_size, recv_size;
	struct sockaddr_in sv_config;
	GamePacket packet;
	GamePacket p1_packet, p2_packet;

	// ソケット作成
	sv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_sock == -1) {
		perror("Error: socket");
		return -1;
	}

	// サーバー設定の初期化
	memset(&sv_config, 0, sizeof(sv_config));
	sv_config.sin_family = AF_INET;
	sv_config.sin_port = htons(SERVER_PORT);
	sv_config.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	// ソケットにIPとPORTを紐づける
	if (bind(sv_sock, (struct sockaddr*)&sv_config, sizeof(sv_config)) == -1) {
		perror("Error: bind");
		close(sv_sock);
		return -1;
	}

	// 接続待ち状態にする
	if (listen(sv_sock, 2) == -1) {
		perror("Error: listen");
		close(sv_sock);
		return -1;
	}

	while (1) {
		memset(&packet, 0, sizeof(GamePacket));
		printf("クライアントの接続を待っています...\n");

		// cl1と接続
		cl_sock[0] = accept(sv_sock, NULL, NULL);
		if (cl_sock[0] == -1) {
			perror("Error: accept");
			continue;
		}
		printf("プレイヤー1が接続されました!\n");

		// cl1に状態を送信
		packet.type = WAIT_PL2;
		send_size = send(cl_sock[0], &packet, sizeof(GamePacket), 0);
		if (send_size < sizeof(GamePacket)) {
			printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(GamePacket));
			close(cl_sock[0]);
		}

		// cl2と接続
		cl_sock[1] = accept(sv_sock, NULL, NULL);
		if (cl_sock[1] == -1) {
			perror("Error: accept");
			continue;
		}
		printf("プレイヤー2が接続されました!\n\n");

		// どちらも接続されたら
		packet.type = INPUT_NAME;
		send_size = send(cl_sock[0], &packet, sizeof(GamePacket), 0);
		if (send_size < sizeof(GamePacket)) {
			printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(GamePacket));
			close(cl_sock[0]);
		}
		send_size = send(cl_sock[1], &packet, sizeof(GamePacket), 0);
		if (send_size < sizeof(GamePacket)) {
			printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(GamePacket));
			close(cl_sock[1]);
		}

		// 名前が送られてくるのを待つ
		recv_size = recv(cl_sock[0], &p1_packet, sizeof(GamePacket), 0);
		if (recv_size <= 0) {
			perror("Error: recv");
			close(cl_sock[0]);
			close(cl_sock[1]);
			return -1;
		}
		recv_size = recv(cl_sock[1], &p2_packet, sizeof(GamePacket), 0);
		if (recv_size <= 0) {
			perror("Error: recv");
			close(cl_sock[0]);
			close(cl_sock[1]);
			return -1;
		}

		printf("プレイヤー1の名前: %s\n", p1_packet.name);
		printf("プレイヤー2の名前: %s\n", p2_packet.name);

		// プレイヤー1へ
		packet.type = GAME_START;
		packet.player_id = 0;
		send(cl_sock[0], &packet, sizeof(GamePacket), 0);

		// プレイヤー2へ
		packet.type = GAME_START;
		packet.player_id = 1;
		send(cl_sock[1], &packet, sizeof(GamePacket), 0);

		printf("対戦処理を開始します\n");
		// ゲーム --------------------------未実装

		close(cl_sock[0]);
		close(cl_sock[1]);
		printf("接続が終了しました。\n");
		break;
	}

	close(sv_sock);
	return 0;
}
