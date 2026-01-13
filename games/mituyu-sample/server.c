#include "mysocket.h"
int createSocket(void);
int listenServer(int, int);
int main(void)
{
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed\n");
		return -1;
	}
#endif
	ConnResCommand resCommand;
	StartCommand startCommand;
	StartResCommand startresCommand;
	int sock_id, recv_sock, recv_size;
	sock_id = createSocket();
	if (sock_id == -1)
	{
		return -1;
	}
	// ポート番号は 27000 + 出席番号
	if (listenServer(sock_id, 27001) == -1)
	{
		close(sock_id);
		return -1;
	}
	while (1)
	{
		recv_sock = accept(sock_id, NULL, NULL);
		if (recv_sock == -1)
		{
			printf("accept error\n");
			close(sock_id);
			break;
		}
		/* 接続の折り返し(ヘッダー+本体)送信 */
		resCommand.header.cmd_id = CONN_RESPONSE;
		resCommand.header.length = sizeof(ConnRes);
		resCommand.body.status = SERVICE_OPEN;
		int send_size = send(recv_sock, (const char *)&resCommand, sizeof(ConnResCommand), 0);
		if (send_size < (int)sizeof(ConnResCommand))
		{
			printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(ConnResCommand));

			close(recv_sock);
			break;
		}
		/* ヘッダー部の受信 */
		recv_size = recv(recv_sock, (char *)&startCommand.header, sizeof(Header), 0);
		if (recv_size <= 0)
		{
			close(recv_sock);
			printf("recv error(startCommand.header)\n");
			break;
		}
		printf("received: id=%d, length=%d\n", startCommand.header.cmd_id, startCommand.header.length);
		/* スタートコマンドの受信(今回はスタート固定) */
		recv_size = recv(recv_sock, (char *)&startCommand.body, sizeof(Start), 0);
		if (recv_size <= 0)
		{
			close(recv_sock);
			printf("recv error(Start Command)\n");
			break;
		}
		printf("received: %s\n", startCommand.body.name);
		/* スタートコマンドの折り返し(ヘッダー) */
		startresCommand.header.cmd_id = ENTRY;
		startresCommand.header.length = sizeof(StartRes);
		startresCommand.body.gameNo = 1; /* ゲーム No. 1 固定 */
		startresCommand.body.order = 1;	 /* 先行固定 */
		send_size = send(recv_sock, (const char *)&startresCommand, sizeof(StartResCommand), 0);
		if (send_size < (int)sizeof(StartResCommand))
		{
			printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(StartResCommand));

			close(recv_sock);
			break;
		}
		close(recv_sock);
	}
	close(sock_id);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
