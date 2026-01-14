#include "common.h"

void game(int sock) {
	
}

int main(void) {
	int sv_sock;
	int cl_sock;
	struct sockaddr_in sv_config;

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
	if (bind(sv_sock, (struct sockaddr *)&sv_config, sizeof(sv_config)) == -1) {
		perror("Error: bind");
		close(sv_sock);
		return -1;
	}

	// 接続待ち状態にする
	if (listen(sv_sock, 3) == -1) {
		perror("Error: listen");
		close(sv_sock);
		return -1;
	}

	while (1) {
		printf("クライアントの接続を待っています...\n");

		// クライアントからの接続を待機
		cl_sock = accept(sv_sock, NULL, NULL);
		if (cl_sock == -1) {
			perror("Error: accept");
			continue;
		}

		printf("接続されました！ (ソケットID: %d)\n\n", cl_sock);

		game(cl_sock);

		close(cl_sock);
		printf("接続が終了しました。\n");
	}

	close(sv_sock);
	return 0;
}
