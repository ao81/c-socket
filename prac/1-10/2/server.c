#include "common.h"

int main(void) {
	int sv_fd, cl_fd[MAX_CLIENTS] = { 0 };
	int i, j, max_fd, added, players_cnt = 0;
	int recv_buf, recv_size, send_buf;
	struct sockaddr_in sv_addr;
	fd_set readfds;

	sv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_fd == -1) {
		perror("Error: socket");
		return -1;
	}

	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(SERVER_PORT);
	sv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(sv_fd, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1) {
		perror("Error: bind");
		close(sv_fd);
		return -1;
	}

	if (listen(sv_fd, MAX_CLIENTS) == -1) {
		perror("Error: listen");
		close(sv_fd);
		return -1;
	}


	while (1) {
		FD_ZERO(&readfds);
		FD_SET(sv_fd, &readfds);
		max_fd = sv_fd;

		for (i = 0; i < MAX_CLIENTS; i++) {
			if (cl_fd[i] > 0) {
				FD_SET(cl_fd[i], &readfds);
			}
			if (cl_fd[i] > max_fd) {
				max_fd = cl_fd[i];
			}
		}

		if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
			perror("Error: select");
		}

		if (FD_ISSET(sv_fd, &readfds)) {
			int new_fd = accept(sv_fd, NULL, NULL);
			if (new_fd >= 0) {
				added = 0;
				for (i = 0; i < MAX_CLIENTS; i++) {
					if (cl_fd[i] == 0) {
						cl_fd[i] = new_fd;
						players_cnt++;
						printf("新規接続: socket %d (現在 %d 人)\n", new_fd, players_cnt);
						added = 1;
						break;
					}
				}
				if (added == 0) {
					printf("満員のため切断しました\n");
					close(new_fd);
				}
			}
		}

		for (i = 0; i < MAX_CLIENTS; i++) {
			if (cl_fd[i] > 0 && FD_ISSET(cl_fd[i], &readfds)) {
				recv_size = recv(cl_fd[i], &recv_buf, sizeof(recv_buf), 0);
				if (recv_size == 0) {
					printf("切断: socket %d\n", cl_fd[i]);
					close(cl_fd[i]);
					cl_fd[i] = 0;
					players_cnt--;
				} else if (recv_size == -1) {
					perror("Error: recv");
					close(cl_fd[i]);
					cl_fd[i] = 0;
					players_cnt--;
				} else {
					printf("受信メッセージ: %d\n", recv_buf);

					send_buf = recv_buf * 2;
					send(cl_fd[i], &send_buf, sizeof(send_buf), 0);


				}
			}
		}
	}

	close(sv_fd);
	return 0;
}