#include <Novice.h>
#include <math.h>
#include <mmsystem.h>
#include <process.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "winmm.lib")

DWORD WINAPI threadfunc(void*);
SOCKET sock;
bool bSocket = false;
HWND hwMain;

const char kWindowTitle[] = "KAMATA ENGINEサーバ";

typedef struct {
	float x;
	float y;
} Vector2;

typedef struct {
	Vector2 center;
	float radius;
} Circle;

// キー入力結果を受け取る箱
Circle a, b;
Vector2 center = {100, 100};
char keys[256] = {0};
char preKeys[256] = {0};
int color = RED;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WSADATA wdData;
	static HANDLE hThread;
	static DWORD dwID;

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 800, 600);

	hwMain = GetDesktopWindow();

	a.center.x = 400;
	a.center.y = 400;
	a.radius = 100;

	b.center.x = 200;
	b.center.y = 200;
	b.radius = 50;

	// winsock初期化
	WSAStartup(MAKEWORD(2, 0), &wdData);

	// データを送受信処理をスレッド（WinMainの流れに関係なく動作する処理の流れ）として生成。
	// データ送受信をスレッドにしないと何かデータを受信するまでRECV関数で止まってしまう。
	hThread = (HANDLE)CreateThread(NULL, 0, &threadfunc, (LPVOID)&a, 0, &dwID);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		if (keys[DIK_UP] != 0) {
			b.center.y -= 5;
		}
		if (keys[DIK_DOWN] != 0) {
			b.center.y += 5;
		}
		if (keys[DIK_RIGHT] != 0) {
			b.center.x += 5;
		}
		if (keys[DIK_LEFT] != 0) {
			b.center.x -= 5;
		}

		///
		/// ↓更新処理ここから
		///

		float distance = sqrtf((float)pow((double)a.center.x - (double)b.center.x, 2) + (float)pow((double)a.center.y - (double)b.center.y, 2));

		if (distance <= a.radius + b.radius) {
			color = BLUE;
		} else
			color = RED;
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		Novice::DrawEllipse((int)a.center.x, (int)a.center.y, (int)a.radius, (int)a.radius, 0.0f, WHITE, kFillModeSolid);
		Novice::DrawEllipse((int)b.center.x, (int)b.center.y, (int)b.radius, (int)b.radius, 0.0f, color, kFillModeSolid);
		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();

	// winsock終了
	closesocket(sock);
	WSACleanup();

	return 0;
}

// 通信スレッド関数
DWORD WINAPI threadfunc(void* px) {

	px;

	int fromlen, recv_cnt, send_cnt;
	struct sockaddr_in addr, recv_addr;

	// ソケット
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	// 8000番に接続待機用ソケット作成
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		return 1;
	}

	// ソケット作成フラグセット
	bSocket = true;

	while (1) {
		fromlen = sizeof(recv_addr);
		recv_cnt = send_cnt = 0;

		// データ受け取り
		recv_cnt = recvfrom(sock, (char*)&a, sizeof(Circle), 0, (struct sockaddr*)&recv_addr, &fromlen);

		while (send_cnt == 0) {
			// メッセージ送信
			send_cnt = sendto(sock, (const char*)&b, sizeof(Circle), 0, (struct sockaddr*)&recv_addr, sizeof(recv_addr));
		}
	}

	return 0;
}