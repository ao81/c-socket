#include "common.h"

int main(void) {
	int sock;
	int recv_size;
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
	struct sockaddr_in sv_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Error: socket");
		return -1;
	}

	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	printf("接続開始...\n");
	if (connect(sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) == -1) {
		perror("Error: connect");
		close(sock);
		return -1;
	}
	printf("接続成功!\n");

	printf("送信する文字列を送信してください: ");
	scanf("%s", send_buf);
	if (send(sock, send_buf, strlen(send_buf), 0) == -1) {
		perror("Error: send");
		close(sock);
		return -1;
	} else {
		memset(recv_buf, 0, sizeof(recv_buf));
		recv_size = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
		if (recv_size == 0) {
			printf("切断\n");
			close(sock);
			return -1;
		} else if (recv_size == -1) {
			perror("Error: recv");
			close(sock);
			return -1;
		} else {
			printf("受信したメッセージ: %s\n", recv_buf);
		}
	}

	close(sock);
	return 0;
}