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
#define TURN_COUNT 4

/* トランクに入る最大金額 */
#define MAX_TRUNK 100000000 /* 1億 */

/* ログイン状態 */
#define NO_LOGIN 0
#define LOGIN 1

/* 通信のタイプ */
typedef enum {
	CONN_NEW,		/* 新規接続 */
	CONN_WAIT,		/* 待ち状態 */
	NAME,			/* sv->cl:名前送信を促す / cl->sv:名前送信 */
	START,			/* ゲーム開始 */
	ACTIONS,		/* アクション */
	END,			/* ゲーム終了 */
} ConnType;

/* アクションのタイプ */
typedef enum {
	WAIT,			/* 互いの選択待ち */
	TRUNK,			/* 密輸者:トランクに入れた金 */
	CHECK,			/* 検査官:トランクの確認 */
	PASS,			/* 検査官:パス */
	DOUBT,			/* 検査官:ダウト */
	ROUND_RESULT,	/* リザルト */
} ActType;

/* ターンごとのリザルトのタイプ */
typedef enum {
    RESULT_PASS,              /* パス：検査官がスルー */
    RESULT_DOUBT_INNOCENT,    /* ダウト失敗：中身が0円だった */
    RESULT_DOUBT_CAUGHT,      /* ダウト成功：中身が予想額以下 */
    RESULT_DOUBT_OVERFLOW     /* ダウト失敗：中身が予想額より多い */
} ResultType;

/* アクション (sv->cl:要求 / cl->sv:入力) */
typedef struct {
	ActType type;		/* アクションタイプ */
	long trunk_amount;	/* 密輸額 */
	long doubt_amount;	/* ダウト宣言額 */
} Actions;

/* 通信電文 */
typedef struct {
	int gameNo;			/* ゲームNo */
	int order;			/* 順番（プレイヤー1:0 / プレイヤー2:1） */
	char name[256];		/* 名前用バッファ */
	Actions action;		/* ゲーム中のデータ */
	ConnType connType;	/* 通信のタイプ */
	int winner;			/* 0:密輸者勝利  1:検査官勝利  2:引き分け */
} Polling;

static void clear_stdin() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}
