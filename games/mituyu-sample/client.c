#include "mysocket.h"

int createSocket(void);
int connectServer(int, char *, int);

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
	StartCommand startCommand;
	StartResCommand startResCommand;
	int sock_id, send_size;
	sock_id = createSocket();
	if (sock_id == -1)
	{
		return -1;
	}
	/* ポート番号は 27000 + 出席番号 */
	if (connectServer(sock_id, "127.0.0.1", 27001) == -1)
	{
		return -1;
	}
	/* 送信データの作成 */
	printf("密輸ゲームにようこそ。プレイヤーの名前を入力してください。\n");
	while (1)
	{
		if (fgets(startCommand.body.name, sizeof(startCommand.body.name),stdin) == NULL)
		{
			printf("プレイヤー名を入力してください。\n");
			continue;
		}
		/* 改行文字を削除 */
		size_t len = strlen(startCommand.body.name);
		if (len > 0 && startCommand.body.name[len - 1] == '\n')
		{
			startCommand.body.name[len - 1] = '\0';
		}
		/* 空白のみの場合も再入力させる。 */
		if (strlen(startCommand.body.name) > 0)
		{
			break;
		}
		printf("プレイヤー名を入力してください。\n");
	}
	startCommand.header.cmd_id = ENTRY;
	startCommand.header.length = sizeof(Start);
	send_size = send(sock_id, (const char *)&startCommand, sizeof(StartCommand), 0);
	if (send_size < (int)sizeof(StartCommand))
	{
		printf("send bytes[%d], original bytes[%zu]\n", send_size, sizeof(StartCommand));
		close(sock_id);
		return -1;
	}
	printf("ゲーム開始中...\n");
	/* スタートコマンドの応答の受信(今回はスタート固定) */
	int recv_size = recv(sock_id, (char *)&startResCommand.header, sizeof(Header), 0);
	if (recv_size <= 0)
	{
		close(sock_id);
		printf("recv error(Start Res Command)\n");
		return -1;
	}
	recv_size = recv(sock_id, (char *)&startResCommand.body, sizeof(StartRes), 0);
	if (recv_size <= 0)
	{
		close(sock_id);
		printf("recv error(Start Res Command body)\n");
		return -1;
	}
	printf("received: GameNo.=%d, Order=%d\n", startResCommand.body.gameNo, startResCommand.body.order);
	close(sock_id);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
