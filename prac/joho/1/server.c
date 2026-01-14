#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

int main(void){
	struct sockaddr_in saddr;
	struct hostent *hp;
	char hostname[256];
	char buffer[256];
	int sock_id, recv_sock, recv_size;
	sock_id = socket(AF_INET, SOCK_STREAM, 0);
	if( sock_id == -1) {
		return -1;
	}

	memset(&saddr, 0x00, sizeof(saddr));
	gethostname(hostname, 256);
	hp = gethostbyname(hostname);
	saddr.sin_port = htons(27001); // ポート番号は 27000 + 出席番号
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
		close(sock_id);
		return -1;
	}
	if( listen(sock_id, 1) != 0 ) {
		close(sock_id);
		return -1;
	}

	while(1) {
		recv_sock = accept(sock_id, NULL, NULL);
		if( recv_sock == -1) {
			printf("accept error\n");
			close(sock_id);
			break;
		}
		recv_size = recv(recv_sock, buffer, sizeof(buffer), 0);
		if(recv_size == 0 || recv_size == -1) {
			close(recv_sock);
			printf("recv error\n");
			break;
		}
		printf("received: %s\n", buffer);

		close(recv_sock);
	}

	return 0;
}
