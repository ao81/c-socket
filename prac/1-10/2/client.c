#include "common.h"

int main(void) {
	int sv_fd;
	struct sockaddr_in sv_addr;
	int send_buf, recv_buf, recv_size;

	sv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_fd == -1) {
		perror("Error: socket");
		return -1;
	}

	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	printf("接続開始...\n");
	if (connect(sv_fd, (struct sockaddr*)&sv_addr, sizeof(struct sockaddr_in)) == -1) {
		perror("Error: connect");
		close(sv_fd);
		return -1;
	}
	printf("接続成功!\n");

	printf("送信する数字を入力してください: ");
	scanf("%d", &send_buf);
	if (send(sv_fd, &send_buf, sizeof(send_buf), 0) < 1) {
		perror("Error: send");
		close(sv_fd);
		return -1;
	} else {
		recv_size = recv(sv_fd, &recv_buf, sizeof(recv_buf), 0);
		if (recv_size == 0) {
			printf("切断: socket %d\n", sv_fd);
		} else if (recv_size == -1) {
			perror("Error: recv");
			close(sv_fd);
			return -1;
		} else {
			printf("受信メッセージ: %d\n", recv_buf);
		}
	}

	close(sv_fd);
	return 0;
}