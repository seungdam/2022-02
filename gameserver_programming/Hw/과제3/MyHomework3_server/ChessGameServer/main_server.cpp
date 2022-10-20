#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WS2tcpip.h>
#include <unordered_map>
#include <Windows.h>
#include <array>
#include <random>

#include "protocol.h"
#pragma comment(lib, "WS2_32.lib")
using namespace std;
constexpr int PORT_NUM = 9000;
constexpr int BUFSIZE = 256;

random_device rd;
default_random_engine dre(rd());
uniform_int_distribution pos(1, 8);
std::array<C_INFO, 10> cinfo;

void err_display(LPSTR msg) {
	LPVOID lpMsgBuf; // 오류 메세지를 담을 버퍼
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::wcout << "[" << msg << "] " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}


void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

class EXP_OVER {
public:
	WSAOVERLAPPED m_over;
	int m_id;
	int m_pos_id;
	WSABUF	m_wsa[1];
	char  m_buff[BUFSIZE];
	EXP_OVER(int client_id, int pos_id) : m_id(client_id), m_pos_id(pos_id)
	{
		m_wsa[0].len = 3;
		m_wsa->buf = m_buff;
		m_buff[0] = m_pos_id;
		printf("%d\n",m_id);
		m_buff[1] = cinfo[m_pos_id - 1].x;
		m_buff[2] = cinfo[m_pos_id - 1].y;
		
		ZeroMemory(&m_over, sizeof(m_over));
		m_over.hEvent = (HANDLE)m_id;
	}
};

class SESSION;
unordered_map<int, SESSION> clients;


class SESSION {
	SOCKET client;
	WSAOVERLAPPED c_over;
	WSABUF c_wsabuf[1];
	int m_id;
public:

	CHAR c_mess[BUFSIZE];

	SESSION() { exit(-1); }
	SESSION(int id, SOCKET so) : m_id(id), client(so)
	{
		c_wsabuf[0].buf = c_mess;
	}
	~SESSION()
	{
		closesocket(client);
	}
	void do_recv()
	{
		c_wsabuf[0].len = 1;
		DWORD recv_flag = 0;
		ZeroMemory(&c_over, sizeof(c_over));
		c_over.hEvent = reinterpret_cast<HANDLE>(m_id);
		int retval = WSARecv(client, c_wsabuf, 1, 0, &recv_flag, &c_over, recv_callback);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("%d\n", WSAGetLastError());
				printf("client[%d] recv_error\n", m_id);
				clients.erase((int)c_over.hEvent);
				cinfo[m_id - 1].is_full = false;
			}
		}

	}
	void do_send(int pos_id)
	{
		EXP_OVER* o = new EXP_OVER(m_id, pos_id);
		int retval = WSASend(client, o->m_wsa, 1, 0, 0, &o->m_over, send_callback);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("client[%d] send_error\n", m_id);
				clients.erase((int)c_over.hEvent);
				cinfo[m_id - 1].is_full = false;
			}
		}
	}
};

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int client_id = reinterpret_cast<int>(over->hEvent);
	int key = clients[client_id].c_mess[0]; // 클라에서 입력된 키보드 키 값
	if (key == VK_UP) {
		if (cinfo[client_id - 1].y > 1) cinfo[client_id - 1].y -= 1;
	}
	if (key == VK_DOWN) {
		if (cinfo[client_id - 1].y <= 7) cinfo[client_id - 1].y += 1;
	}
	if (key == VK_RIGHT) {
		if (cinfo[client_id - 1].x <= 7) cinfo[client_id - 1].x += 1;
	}
	if (key == VK_LEFT) {
		if (cinfo[client_id - 1].x > 1) cinfo[client_id - 1].x -= 1;
	}

	// 다른 클라이언트에게 모두 전송
	for (auto& cl : clients) {
		cl.second.do_send(client_id);
	}
	clients[client_id].do_recv();
}


void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	delete over;
}

int main()
{

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) return -1;
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	for (auto& i : cinfo) {
		i.is_full = false;
		i.x = 0;
		i.y = 0;
	}
	
	for (;;) {
		SOCKET client = WSAAccept(server,
		reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, NULL, NULL);
		int idx = 1;
		for (auto& i : cinfo) {
			if (!i.is_full) {
				i.x = pos(dre);
				i.y = pos(dre);
				i.is_full = true;
				clients.try_emplace(idx, idx, client);
				printf("[%d] clint Enter\n", idx);
				clients[idx].do_recv();
				break;
			}
			++idx;
		}
		if (11 == idx) closesocket(client);
		
	}

	closesocket(server);
	WSACleanup();
}

