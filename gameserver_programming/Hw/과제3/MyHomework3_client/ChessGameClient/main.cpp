#include <winsock2.h>
#include <WS2tcpip.h>
#include <regex>
#include <string.h>
#include "resource.h"
#include "ChessGame.h"

#define SERVER_PORT 9000
#define BUFSIZE 256
#pragma comment(lib,"ws2_32")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK IpDlgProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_hInst;
HWND g_hWnd;
LPCTSTR lpszClass = TEXT("HomeWork3");
CChessGame maingame;

WSAOVERLAPPED s_over;
WSAOVERLAPPED r_over;
SOCKET s_sock;
WSABUF s_wsabuf;
WSABUF s_wsabuf2;
char   s_buf[BUFSIZE];
char   s_buf2[BUFSIZE];
WSADATA wsa;
sockaddr_in serveraddr;
TCHAR ipaddr[50] = TEXT("127.0.0.1");

void do_send_message(int);
void do_recv_message();
void CALLBACK recv_callback(DWORD , DWORD , LPWSAOVERLAPPED , DWORD );
void CALLBACK send_callback(DWORD , DWORD , LPWSAOVERLAPPED , DWORD );

void err_quit(LPCWSTR msg) {
	LPVOID lpMsgBuf; // 오류 메세지를 담을 버퍼
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBoxW(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);

}

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	wprintf(L"%d\n",WSAGetLastError());
	LocalFree(lpMsgBuf);
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;   // 윈도우 핸들
	MSG Message; // 메세지
	g_hInst = hInstance; // 인스턴스 핸들을 다른 프로세스에서도 사용할 수 있도록 전역변수에 대입

	// 윈도우 클래스(윈도우의 속성을 정의하는 구조체)를 초기화
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpszClassName = lpszClass;
	wc.lpfnWndProc = WndProc;
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClass(&wc); // 초기화 시킨 윈도우 클래스를 운영체제에 등록

	// 윈도우 클래스 바탕으로 윈도우창을 생성한다
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 610, 630
		, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow); // 윈도우창을 출력
	if (hWnd == NULL) return -1;
	g_hWnd = hWnd;
	// 윈속 2.2로 초기화
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) return -1;
	// 클라이언트 소켓 생성
	s_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s_sock) err_quit(TEXT("socket()"));
	if(IDCANCEL == DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)IpDlgProc)) //return 0;
	// 소켓 주소 구조체 초기화.
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	InetPtonW(AF_INET, (PCWSTR)ipaddr, &serveraddr.sin_addr.s_addr);
	
	maingame.InitStage(g_hInst);
	// connect
	int retval = 0;
	retval = WSAConnect(s_sock, (sockaddr*)&serveraddr, sizeof(serveraddr),0,0,0,0);
	if (retval == SOCKET_ERROR) err_quit(TEXT("WSAConnect()"));
	do_recv_message();
	
	do_send_message(-1);
	// 메세지 루프
	while (true) {
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE) > 0) { // 메세지 큐에 있는 메세지들을 확인
			if (Message.message == WM_QUIT) break;
			SleepEx(1, true);
			TranslateMessage(&Message); // 키보드에 입력된 메세지를 인스턴스가 이해하기 쉬운 형태로 해석
			DispatchMessage(&Message); // WndProc으로 해석시킨 메세지를 전달한다.
		}
		InvalidateRect(g_hWnd, NULL, FALSE);
	}

	// 소켓 닫기
	closesocket(s_sock);
	// 윈속 사용 중지
	WSACleanup();

	return (int)Message.wParam;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	
	switch (iMessage) {
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_SPACE:
			break;
		case VK_DOWN:
		case VK_UP:
		case VK_RIGHT:
		case VK_LEFT:
			do_send_message(wParam);
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_PAINT:
		
		hdc = BeginPaint(hWnd, &ps);
		maingame.DrawScreen(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}


BOOL CALLBACK IpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	std::wregex ip_re(TEXT("[0-9]{1,3}(\\.)[0-9]{1,3}(\\.)[0-9]{1,3}(\\.)[0-9]{1,3}"));
	std::wstring wstr;
	BOOL m = false;
	switch (iMessage) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT_IPADDR, ipaddr, 50);
			 wstr = ipaddr;
			 m = std::regex_match(wstr, ip_re);
			if (m) {
				EndDialog(hDlg, IDOK);
				return TRUE;
			}
			 return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

void do_send_message(int key) {
	ZeroMemory(&s_buf, sizeof(s_buf));
	ZeroMemory(&s_over, sizeof(OVERLAPPED));

	s_wsabuf.buf = s_buf;
	s_wsabuf.len = 1;
	s_buf[0] = key;

	int retval = WSASend(s_sock, &s_wsabuf, 1, 0, 0, &s_over, send_callback);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			PostQuitMessage(0);
		}
	}
}

void do_recv_message() {
	ZeroMemory(&s_buf2, sizeof(s_buf2));
	ZeroMemory(&r_over, sizeof(OVERLAPPED));

	s_wsabuf2.buf = s_buf2;
	s_wsabuf2.len = 3;
	
	DWORD r_flag = 0;
	int retval = WSARecv(s_sock, &s_wsabuf2, 1, 0, &r_flag, &r_over, recv_callback);
	if (retval == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			PostQuitMessage(0);
		}
	}
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	
	char* p = s_buf2;
	int receive_byte = 3;
	int c_id = p[0];
	while (receive_byte > 0) {
		receive_byte -= num_bytes;
	}
	printf("%d %d %d\n", c_id, s_buf2[1], s_buf2[2]);
	maingame.MovePlayer(c_id, p[1], p[2]);
	do_recv_message();
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	ZeroMemory(&s_buf, sizeof(s_buf));
	ZeroMemory(over, sizeof(OVERLAPPED));
}