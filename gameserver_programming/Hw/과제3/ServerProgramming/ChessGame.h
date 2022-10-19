#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <array>
#include <random>

#include "resource.h"

// #pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib,"msimg32.lib")
#define MAP_COL 8
#define MAP_ROW  8




class CP {
public:
	int x;
	int y;
	CP() :x(0), y(0) {};
	CP(int a, int b) : x(a), y(b) {};
	~CP() {};
};

class CChessGame
{

	int nPlayer = 0;
	HBITMAP rc = NULL; // ¸ÊÀÌ¶û ÇÃ·¹ÀÌ¾î
	HBRUSH ob = NULL;
	HBRUSH nb = NULL;
public:
	std::array<CP,10> mp;
	CChessGame() {
		nb = CreateSolidBrush(RGB(255, 0, 0));
		for (int i = 0; i < 10; ++i) {
			mp[i].x = 0;
			mp[i].y = 0;
		}
	};
	void MovePlayer(int id, int x, int y) { mp[id - 1].x = x; mp[id - 1].y = y; };
	void InitStage(HINSTANCE&);
	void DrawScreen(HDC);
	void DrawBitmap(HDC, HBITMAP&, int, int);

	~CChessGame() { DeleteObject(nb); };
};

