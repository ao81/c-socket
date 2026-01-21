#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 57001
#define BUF_SIZE 1024

// 通信の種類
typedef enum {
	SEND, // 送信
	WAIT_PL1, // プレイヤー1待ち
	WAIT_PL2, // プレイヤー2待ち
	INPUT_NAME, // プレイヤー名入力
	GAME_START, // ゲーム開始
	ACT, // 行動要求
	WAIT, // 待機通知
	SMUGGLE, // 密輸アクション
	INSPECT, // 検査アクション
	RESULT, // ラウンド結果通知
	GAME_OVER // ゲーム終了
} MessageType;

// クライアント（検査官）のアクション
typedef enum {
	PASS, // パス
	DOUBT // ダウト
} InspectionType;

typedef struct {
	MessageType type; // メッセージの種類

	int player_id; // 0:北, 1:南
	char name[256]; // プレイヤーの名前

	// ゲームデータ
	long long current_money; // 現在の所持金
	long long atm_balance; // ATM残高

	// アクション用データ
	long long truck_amount; // 密輸額
	InspectionType action; // パス or ダウト
	long long doubt_amount; // ダウト宣言額

	// 結果通知用データ
	int is_smuggling_success; // 1:成功, 0:失敗
} GamePacket;
