#pragma once
#include "Global.h"
#include "GSEObjectMgr.h"
#include "Renderer.h"

class GSEGame
{
	GSEObjectMgr* m_objectMgr;
	Renderer* m_renderer;
	int m_hero_id = -1;
public:
	GSEGame() { printf("CGame cannot exist"); exit(1); };
	GSEGame(GSEVec2);
	void RenderScene(); // elapsed time �� �پ��� �Ķ���Ͱ� ���� ����
	void UpdateObjects(GSEKeyBoardMapper,float);
	~GSEGame();
};

