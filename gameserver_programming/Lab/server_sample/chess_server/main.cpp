#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <concurrent_unordered_set.h> // non blocking set���� ��ŷ�� �ʿ䰡 ��������.
// ������ ���⼭ add �� remove�� ����ŷ���� ���ư����� '='�� ����ŷ�� �ƴϴ�. --> ī�Ǹ� ���� ����ȭ �ϴ� ���� �Ұ����� ����.
// �� add remove�� �ݵ�� �������� ����.
#include <unordered_set>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;
constexpr int MAX_USER = 30;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum S_STATE  { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
	OVER_EXP _recv_over;

public:
	mutex _s_lock;
	S_STATE _state;
	int _id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	std::unordered_set<int> viewlist;
	mutex _vl;
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}
	void send_add_player(int c_id);
	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}
	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_remove_player_packet(int c_id) {
		_vl.lock();
		if(viewlist.count(c_id)) viewlist.erase(c_id);
		else {
			_vl.unlock();
			return;
		}
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	};
};

array<SESSION, MAX_USER> clients;
SOCKET g_s_socket, g_c_socket;
OVER_EXP g_a_over;

bool can_see(int from, int to) //  �þ߿� �ֺ� Ŭ���̾�Ʈ�� ������ ���� �Ǵ��Ѵ�.
{
	if (abs(clients[from].x - clients[to].x) > VIEW_RANGE) return false;
	if (abs(clients[from].y - clients[to].y) > VIEW_RANGE) return false;

	return true;
}

void SESSION::send_add_player(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.x = clients[c_id].x;
	add_packet.y = clients[c_id].y;

	// �� ����Ʈ�� �߰�.
	_vl.lock();
	viewlist.insert(c_id);
	_vl.unlock();
	do_send(&add_packet);
}

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	_vl.lock();
	viewlist.insert(c_id);
	_vl.unlock();
	do_send(&p);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
		case CS_LOGIN: {
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
			strcpy_s(clients[c_id]._name, p->name);
			clients[c_id].send_login_info_packet(); // �α��� ���� �� �α��� ���� ������ ����.
			{
				lock_guard<mutex> ll{ clients[c_id]._s_lock };
				clients[c_id]._state = ST_INGAME;
			}
			// ���� �α����� ����� ���� �ٸ� �÷��̾�� �˷��ش�.
			for (auto& pl : clients) {
				{
					lock_guard<mutex> ll (pl._s_lock);
					if (ST_INGAME != pl._state) continue;
				}
				if (pl._id == c_id) continue;
				// 2022-10-18 �þ�ó��
				if (false == can_see(c_id, pl._id)) continue; // �þ߿� �� ���� �ʴٸ� �������� �ʴ´�.

				//SC_ADD_PLAYER_PACKET add_packet;
				//add_packet.id = c_id;
				//strcpy_s(add_packet.name, p->name);
				//add_packet.size = sizeof(add_packet);
				//add_packet.type = SC_ADD_PLAYER;
				//add_packet.x = clients[c_id].x;
				//add_packet.y = clients[c_id].y;
				//
				//// �� ����Ʈ�� �߰�.
				//pl._vl.lock();
				//pl.viewlist.insert(c_id);
				//pl._vl.unlock();
				//pl.do_send(&add_packet);
				pl.send_add_player(c_id); // ���� �����ؼ� ���⼭ �ѹ��� ����. 10-18
				clients[c_id].send_add_player(c_id);
			}
			// ���� ���縦 ���濡�� �˷�����Ѵ� .10-18
			for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = pl._id;
			strcpy_s(add_packet.name, pl._name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.x = pl.x;
			add_packet.y = pl.y;
			clients[c_id].do_send(&add_packet);
		}
			break;
		}
		case CS_MOVE: {
			CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
			short x = clients[c_id].x;
			short y = clients[c_id].y;
			switch (p->direction) {
			case 0: if (y > 0) y--; break;
			case 1: if (y < W_HEIGHT - 1) y++; break;
			case 2: if (x > 0) x--; break;
			case 3: if (x < W_WIDTH - 1) x++; break;
			}
			clients[c_id].x = x;
			clients[c_id].y = y;
			
			unordered_set<int> near_list; // �� �ֺ��� ���� �ִ°� 10 -18
			// ����ȭ�� ���� copy ����� ���.
			clients[c_id]._vl.lock();
			unordered_set<int> old_vList = clients[c_id].viewlist;
			clients[c_id]._vl.lock();

			for (auto& cl : clients) {
				if (cl._state != ST_INGAME) continue;
				if (cl._id == c_id) continue; // �� �ڽ��� �丮��Ʈ�� ������ �ȵ�
				if (can_see(c_id, cl._id)) near_list.insert(cl._id);
			}
			clients[c_id].send_move_packet(c_id);
			// �ֺ� �÷��̾�Ը� ��������.
			
			for (auto& pl : near_list) {
				auto& cpl = clients[pl];
				cpl._vl.lock();
				if (clients[pl].viewlist.count(c_id)) {
					cpl._vl.unlock();
					clients[pl].send_move_packet(c_id);
				}
				else {
					cpl._vl.unlock();
					clients[pl].send_add_player(c_id); // ���濡�� �־��ش�.
					                           
				}
				// ������ near list�� �ִ� ������ ������ ���ٸ� �����Ե� ������ �־��ش�.
				
				if (old_vList.count(pl) == 0) {
					clients[c_id].send_add_player(pl);
				}
			}
			for (auto& pl : old_vList) {
				if (0 == near_list.count(pl)) {
					clients[c_id].send_remove_player_packet(pl);
					clients[pl].send_remove_player_packet(c_id);
				}
			}
			// ��� Ŭ���̾�Ʈ�� ���ؼ� ������������.
			/*	for (auto& pl : clients) {
				{
					lock_guard<mutex> ll(pl._s_lock);
					if (ST_INGAME != pl._state) continue;
				}
				pl.send_move_packet(c_id);
			}*/
			break;
		}
	}
}

void disconnect(int c_id)
{
	clients[c_id]._vl.lock();
	unordered_set<int> vl = clients[c_id].viewlist;
	clients[c_id]._vl.unlock();

	for(auto& pl_id : vl) {
	//for (auto& pl : clients) {
		auto& pl = clients[pl_id];
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		/*SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);*/
		pl.send_remove_player_packet(c_id);
	}
	closesocket(clients[c_id]._socket);

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	clients[c_id]._state = ST_FREE;
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket),
					h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		}
	}
}

int main()
{
	HANDLE h_iocp;

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);
	for (auto& th : worker_threads)
		th.join();
	closesocket(g_s_socket);
	WSACleanup();
}
