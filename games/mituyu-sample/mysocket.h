#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

/* 操作コマンド */
#define CONN_RESPONSE 0 /* 接続の折り返し */
#define ENTRY 1			/* エントリー */
#define TRADE 2			/* 密輸 */
#define CHECK 3			/* 検査 */
#define POLLING 4		/* ポーリング */
#define GAME_END 5		/* ゲーム終了 */

/* ポーリングの状況コマンド */
#define POLL_WAIT_ENTRY 1	/* 相手受付待ち */
#define POLL_WAIT_TRADE 2	/* 密輸待ち */
#define POLL_WAIT_CHECK 3	/* 検査判定待ち */
#define POLL_WAIT_END 4		/* ゲーム終了待ち */
#define POLL_INS_TRADE 5	/* 密輸指示 */
#define POLL_INS_CHECK 6	/* 検査指示 */
#define POLL_CHECK_RESULT 7 /* 検査結果 */

/* サービス状態 */
#define SERVICE_CLOSE 0 /* サービス非公開 */
#define SERVICE_OPEN 1	/* サービス公開 */

/* 通信電文共通ヘッダー */
typedef struct
{
	int cmd_id; /* コマンド ID */
	int length; /* データ部の長さ */
} Header;

/* 通信電文(接続の折り返し) */
typedef struct
{
	int status; /* システムの状態 */
} ConnRes;

/* 通信電文(スタート) */
typedef struct
{
	char name[256]; /* プレイヤー名 */
} Start;

/* 通信電文(スタートの応答) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
} StartRes;

/* 通信電文(ポーリング) */
typedef struct
{
	int gameNo; /* ゲーム No. */
} Polling;

/* 通信電文(ポーリングの応答) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
	int type;	/* ポーリングの状況コマンド */
} PollingRes;

/* 通信電文(密輸) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
	int money;	/* 密輸金額 */
} Trade;

/* 通信電文(密輸の応答) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
	int status; /* 1:OK、0:NG */
} TradeRes;

/* 通信電文(検査) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
	int money;	/* 密輸金額(0:パス/1 以上:ダウト**円) */
} Check;

/* 通信電文(検査の応答) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
	int money;	/* 増減金額(1 以上:ダウト成功/0 未満:ダウト失敗/0:パス) */
} CheckRes;

/* 通信電文(終了) */
typedef struct
{
	int gameNo; /* ゲーム No. */
	int order;	/* 順番(先攻:1/後攻:2) */
} GameEnd;

/* 通信電文(終了の応答) */
typedef struct
{
	char player1[256]; /* プレイヤー名 1 */
	int money1;		   /* 獲得金額 1 */
	char player2[256]; /* プレイヤー名 2 */
	int money2;		   /* 獲得金額 2 */
} GameEndRes;

/* 接続の折り返しコマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	ConnRes body;  /* ボディ部 */
} ConnResCommand;

/* スタートコマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	Start body;	   /* ボディ部 */
} StartCommand;

/* スタートの応答コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	StartRes body; /* ボディ部 */
} StartResCommand;

/* ポーリングコマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	Polling body;  /* ボディ部 */
} PollingCommand;

/* ポーリングの応答コマンド */
typedef struct
{
	Header header;	 /* ヘッダー部 */
	PollingRes body; /* ボディ部 */
} PollingResCommand;

/* 密輸コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	Trade body;	   /* ボディ部 */
} TradeCommand;

/* 密輸の応答コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	TradeRes body; /* ボディ部 */
} TradeResCommand;

/* 検査コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	Check body;	   /* ボディ部 */
} CheckCommand;

/* 検査の応答コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	Check body;	   /* ボディ部 */
} CheckResCommand;

/* 終了コマンド */
typedef struct
{
	Header header; /* ヘッダー部 */
	GameEnd body;  /* ボディ部 */
} GameEndCommand;

/* 終了の応答コマンド */
typedef struct
{
	Header header;	 /* ヘッダー部 */
	GameEndRes body; /* ボディ部 */
} GameEndResCommand;
