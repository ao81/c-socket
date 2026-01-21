#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 57801
#define BUF_SIZE 1024

// 通信の種類
typedef enum {
	// 接続・準備フェーズ
	WAIT_MATCHING,		// プレイヤーが揃うのを待機
	INPUT_NAME,			// プレイヤー名入力
	GAME_START,			// ゲーム開始通知

	// ターン開始フェーズ
	NOTIFY_ROLE,		// ロールを通知 (smuggle or inspect)

	// 密輸フェーズ（攻め）
	REQUEST_SMUGGLE,	// いくらトランクに入れるか (sv -> smuggle)
	SUBMIT_SMUGGLE,		// 金額 (smuggle -> sv)

	// 検査フェーズ（守り）
	REQUEST_INSPECT,	// パスかダウトか (sv -> inspect)
	SUBMIT_INSPECT,		// 金額 (inspect -> sv)

	// 結果通知フェーズ
	ROUND_RESULT,		// トランクの中身・判定・所持金の変動を通知 (sv -> cl)

	// 状態管理
	NEXT_ROUND,			// 次のラウンドへ
	GAME_OVER,			// 最終結果通知

	// エラー・切断
	ERROR_DISCONNECT,	// 相手が切断したなど
} MessageType;

// 検査官(inspect) のアクション
typedef enum {
	PASS, // パス
	DOUBT // ダウト
} InspectionType;

typedef struct {
	MessageType type; // メッセージの種類

	int player_id; // 0:北, 1:南
	char name[256]; // プレイヤーの名前

	// ゲーム全体データ
	int round_count;			// 現在の回数 (1〜50回)
	long long current_money;	// 現在の所持金（口座残高）
	long long atm_balance;		// 自分の国のATM残高（初期値360億）

	// アクション用データ
	long long trunk_amount;		// トランクに入れた金額 (0〜1億)
	InspectionType action;		// 検査官の選択 (PASS or DOUBT)
	long long doubt_amount;		// ダウト宣言額
	long long deposit_amount;	// 保証金（doubt_amountの半分、自動計算用）

	// 結果通知用データ
	int result_code;
	// 0: パスで密輸成功
	// 1: ダウト的中（宣言以下）で検察官の勝ち
	// 2: ダウト失敗（空だった）で密輸者の勝ち（慰謝料）
	// 3: ダウト失敗（宣言より多い）で密輸者の勝ち（保証金没収）
	long long money_diff;		// 
} GamePacket;
