#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 65501
#define BUF_SIZE 256

/* ターン数 */
#define TURN_COUNT 50

/* トランクに入る最大金額 */
#define MAX_TRUNK 100000000 /* 1億 */

/* ログイン状態 */
#define NO_LOGIN 0
#define LOGIN 1

/* 通信のタイプ */
typedef enum {
	NEW_CONNECT,	/* 新規接続 */
	WAIT,			/* 待ち状態 */
	NAME,			/* sv->cl:名前送信を促す / cl->sv:名前送信 */
	START,			/* ゲーム開始 */
	ACTIONas,		/* アクション */
} ConnType;

/* 検査官のアクション */
typedef enum {
	PASS,	/* パス */
	DOUBT,	/* ダウト */
} Inspection;

/* アクション */
typedef struct {
	Inspection inspection;	/* 検査官のアクション */
	long doubt_amount;		/* ダウト宣言額 */
	long trunk_amount;		/* 密輸額 */
} Actions;

/* 通信電文 */
typedef struct {
	int gameNo;			/* ゲームNo */
	int order;			/* 順番（先行:1 / 後攻:2） */
	char name[256];		/* 名前用バッファ */
	Actions action;		/* ゲーム中のデータ */
	ConnType connType;	/* 通信のタイプ */
} Polling;
