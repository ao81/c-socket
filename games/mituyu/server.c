#include "common.h"

#define MAX_CLIENTS 2

int main(void) {
	int sv_sock;
	int cl_sock[MAX_CLIENTS]; // クライアントのソケット一覧
	char names[MAX_CLIENTS][256]; // クライアントの名前一覧
	int is_name_set[MAX_CLIENTS]; // 名前がセットされたかどうかのフラグ
	struct sockaddr_in sv_config;
	int i, j;
	int max_sd, activity;
	int num_players = 0;

	// 変数の初期化
	for (i = 0; i < MAX_CLIENTS; i++) {
		cl_sock[i] = 0;
		memset(names[i], 0, sizeof(names[i]));
		is_name_set[i] = 0;
	}

	// ソケット作成
	sv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_sock == -1) {
		perror("Error: socket");
		return -1;
	}

	// サーバー設定
	memset(&sv_config, 0, sizeof(sv_config));
	sv_config.sin_family = AF_INET;
	sv_config.sin_port = htons(SERVER_PORT);
	sv_config.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	// bind
	if (bind(sv_sock, (struct sockaddr*)&sv_config, sizeof(sv_config)) == -1) {
		perror("Error: bind");
		close(sv_sock);
		return -1;
	}

	// listen
	if (listen(sv_sock, MAX_CLIENTS) == -1) {
		perror("Error: listen");
		close(sv_sock);
		return -1;
	}

	printf("サーバーが起動しました。接続待機中...\n");

	fd_set readfds;

	while (1) {
		// fd_setの初期化
		FD_ZERO(&readfds);
		FD_SET(sv_sock, &readfds);
		max_sd = sv_sock;

		// クライアントソケットをセットに追加
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (cl_sock[i] > 0) {
				FD_SET(cl_sock[i], &readfds);
			}
			if (cl_sock[i] > max_sd) {
				max_sd = cl_sock[i];
			}
		}

		// select実行
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR)) {
			perror("Error: select");
		}

		// 新しい接続がある場合
		if (FD_ISSET(sv_sock, &readfds)) {
			int new_sock = accept(sv_sock, NULL, NULL);
			if (new_sock < 0) {
				perror("Error: accept");
			} else {
				// 空いている場所を探す
				int added = 0;
				for (i = 0; i < MAX_CLIENTS; i++) {
					if (cl_sock[i] == 0) {
						cl_sock[i] = new_sock;
						// 名前情報などをリセット
						memset(names[i], 0, sizeof(names[i]));
						is_name_set[i] = 0;

						num_players++;
						printf("新しい接続: socket %d (場所: %d, 現在 %d 人)\n", new_sock, i, num_players);
						added = 1;

						GamePacket packet;
						memset(&packet, 0, sizeof(GamePacket));

						// 1人目なら待機させる
						if (num_players == 1) {
							packet.type = WAIT_PL2;
							send(cl_sock[i], &packet, sizeof(GamePacket), 0);
						}

						// 2人揃ったら名前入力を促す
						if (num_players == MAX_CLIENTS) {
							printf("2人揃いました。名前入力を要求します。\n");
							packet.type = INPUT_NAME;
							for (j = 0; j < MAX_CLIENTS; j++) {
								send(cl_sock[j], &packet, sizeof(GamePacket), 0);
							}
						}
						break;
					}
				}

				if (added == 0) {
					printf("満員なので切断しました\n");
					close(new_sock);
				}
			}
		}

		// 各クライアントからのデータ受信確認
		for (i = 0; i < MAX_CLIENTS; i++) {
			int sd = cl_sock[i];

			if (sd > 0 && FD_ISSET(sd, &readfds)) {
				GamePacket packet;
				int recv_size = recv(sd, &packet, sizeof(GamePacket), 0);

				if (recv_size <= 0) {
					// 切断処理（関数を使わずそのまま書く）
					printf("クライアント切断: socket %d\n", sd);
					close(sd);
					cl_sock[i] = 0; // その場所を0に戻すだけ（詰めない）
					is_name_set[i] = 0;
					num_players--;
					printf("現在の人数: %d\n", num_players);

					// 1人になったら残った人を待機状態に戻す
					if (num_players == 1) {
						for (j = 0; j < MAX_CLIENTS; j++) {
							if (cl_sock[j] > 0) {
								GamePacket wait_packet;
								memset(&wait_packet, 0, sizeof(GamePacket));
								wait_packet.type = WAIT_PL2;
								send(cl_sock[j], &wait_packet, sizeof(GamePacket), 0);
							}
						}
					}

				} else {
					// データ受信時の処理
					if (packet.type == SEND && is_name_set[i] == 0) {
						strncpy(names[i], packet.name, sizeof(names[i]) - 1);
						is_name_set[i] = 1;
						printf("socket %d の名前: %s\n", sd, names[i]);

						// 全員名前が決まったかチェック
						int all_ok = 1;
						if (num_players < MAX_CLIENTS) {
							all_ok = 0;
						} else {
							for (j = 0; j < MAX_CLIENTS; j++) {
								if (is_name_set[j] == 0) {
									all_ok = 0;
									break;
								}
							}
						}

						if (all_ok) {
							printf("全員準備完了。ゲーム開始！\n");
							GamePacket start_packet;
							memset(&start_packet, 0, sizeof(GamePacket));
							start_packet.type = GAME_START;

							for (j = 0; j < MAX_CLIENTS; j++) {
								start_packet.player_id = j; // 配列のインデックスをそのままIDにする
								send(cl_sock[j], &start_packet, sizeof(GamePacket), 0);
							}
						}
					}
				}
			}
		}
	}

	close(sv_sock);
	return 0;
}