#include "common.h"

void game(int sock) {
	StartData startData;

	printf("密輸ゲームへようこそ。プレイヤーの名前を入力してください。\n");
	
	while (1) {
		if (fgets(startData.name, sizeof(startData.name), stdin) == NULL) {
			printf("プレイヤー名を入力してください。\n");
			continue;
		}

		send(
	}
}

int main(void) {
	int sock;
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
	if (connect(sock, (struct sockaddr *)&sv_config, sizeof(struct sockaddr_in)) == -1) {
		perror("Error: connect");
		close(sock);
		return -1;
	}
	printf("接続されました！\n");

	game(sock);

	close(sock);
	return 0;
}
