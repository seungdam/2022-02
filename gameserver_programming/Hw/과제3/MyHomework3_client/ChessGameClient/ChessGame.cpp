#include "ChessGame.h"

void CChessGame::InitStage(HINSTANCE& hInst) { // Ȥ�� ���߿� ���̺� �� �� �� ������ ����������� ���� �����ϴ� ����� ����.
	// �÷��̾� �ʱ� ��ġ�� ����.
	rc = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1 + 1));
}


void CChessGame::DrawBitmap(HDC hdc, HBITMAP& hBit, int x, int y)
{
	HDC memDC = CreateCompatibleDC(hdc);
	BITMAP bit;
	HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);
	GetObject(hBit, sizeof(BITMAP), &bit);
	BitBlt(hdc, x, y, bit.bmWidth, bit.bmHeight, memDC, 0, 0, SRCCOPY);
	SelectObject(memDC, oldBit);
	DeleteDC(memDC);
}



void CChessGame::DrawScreen(HDC hdc)
{
	DrawBitmap(hdc, rc, 0, 0);
	HBRUSH ob;
	ob = (HBRUSH)SelectObject(hdc, nb);
	for (auto& i : mp)
		if (i.x > 0) Ellipse(hdc, (i.x - 1) * 75, (i.y - 1) * 75, (i.x - 1) * 75 + 75, (i.y - 1) * 75 + 75);
	SelectObject(hdc, ob);
}
