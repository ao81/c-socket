#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 65001
#define BUF_SIZE 256

/* ターン数 */
#define TURN_COUNT 50

/* ログイン状態 */
#define NO_LOGIN 0
#define LOGIN 1

/* 通信のタイプ */
typedef enum {
	NEW_CONNECT,	/* 新規接続時 */
	WAIT,			/* 待ち状態 */
	NAME,			/* sv->cl:名前送信を促す / cl->sv:名前送信 */
} Type;

/* 通信電文 */
typedef struct {
	int gameNo;		/* ゲームNo */
	int order;		/* 順番（先行:1 / 後攻:2） */
	char name[256];	/* 名前用バッファ */
	Type type;		/* 通信のタイプ */
} Polling;
