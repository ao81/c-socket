#include "common.h"

#define MAX_CLIENTS 10

int main(void) {
	int sv_fd, cl_fd[MAX_CLIENTS] = { 0 }, max_fd;
	int i, recv_size;
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
	fd_set readfds;
	socklen_t cl_len;
	struct sockaddr_in sv_addr, cl_addr;

	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	sv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_fd == -1) {
		perror("Error: socket");
		return -1;
	}

	int opt = 1;
	if (setsockopt(sv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("Error: setsockopt");
		close(sv_fd); // socket() 以降は close() が必要
		return -1;
	}

	if (bind(sv_fd, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) == -1) {
		perror("Error: bind");
		close(sv_fd);
		return -1;
	}

	if (listen(sv_fd, MAX_CLIENTS) == -1) {
		perror("Error: listen");
		close(sv_fd);
		return -1;
	}

	printf("サーバー起動\n\n");

	while (true) {
		FD_ZERO(&readfds);

		FD_SET(sv_fd, &readfds); // 新規接続を監視するため
		max_fd = sv_fd;

		for (i = 0; i < MAX_CLIENTS; i++) {
			int fd = cl_fd[i];
			if (fd == MAX_CLIENTS) {
				printf("接続数上限 fd: %d\n", fd);
				close(fd);
			}
			if (fd > 0) { // クライアントが接続中なら
				FD_SET(fd, &readfds);
			} 
			if (fd > max_fd){
				max_fd = fd;
			}
		}

		int active = select(max_fd + 1, &readfds, NULL, NULL, NULL);
		if (active < 0) {
			perror("Error: select");
			continue;
		}

		// 新規接続
		if (FD_ISSET(sv_fd, &readfds)) {
			cl_len = sizeof(cl_addr);
			int new_fd = accept(sv_fd, (struct sockaddr *)&cl_addr, &cl_len);
			if (new_fd == -1) {
				perror("Error: accept");
			} else {
				printf("新規接続 fd: %d\n", new_fd);
				for (i = 0; i < MAX_CLIENTS; i++) {
					if (cl_fd[i] == 0) {
						cl_fd[i] = new_fd;
						break;
					}
				}
			}
		}

		for (i = 0; i < MAX_CLIENTS; i++) {
			int fd = cl_fd[i];

			if (fd > 0 && FD_ISSET(fd, &readfds)) {
				memset(recv_buf, 0, sizeof(recv_buf));
				recv_size = recv(fd, recv_buf, sizeof(recv_buf), 0);

				if (recv_size == 0 || errno == ECONNRESET) {
					printf("切断 fd: %d\n", fd);
					close(fd);
					cl_fd[i] = 0;
				} else if (recv_size == -1) {
					perror("Error: recv");
					close(fd);
					cl_fd[i] = 0;
				} else {
					recv_buf[recv_size] = '\0';
					printf("受信（fd: %d）: %s\n", cl_fd[i], recv_buf);

					// そのまま帰す
					send(fd, recv_buf, recv_size, 0);
				}
			}
		}
	}

	close(sv_fd);
	return 0;
}