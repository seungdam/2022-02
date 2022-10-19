#include <limits>
#include <random>
#include "stdafx.h"
#include "GSEGame.h"
#include "GSEObject.h"

GSEGame::GSEGame(GSEVec2 size)
{
	m_renderer = new Renderer((int)size.x,(int)size.y); // x,y는 윈도우 사이즈
	m_objectMgr = new GSEObjectMgr();

	//Create Hero Object
	GSEVec3 heroObjPos{ 0,0,0 };
	GSEVec3 heroObjSize{ 100,10,10 };
	GSEVec3 heroObjVel{ 0,0,0 };
	GSEVec3 heroObjAcc{ 0,0,0 };
	float heroObjMass = 1.f;
	m_hero_id = m_objectMgr->AddObject(heroObjPos, heroObjSize, heroObjVel, heroObjAcc, heroObjMass);

	GSEVec3 objPos{ 0,0,0 };
	GSEVec3 objSize{ 10,10,10 };
	GSEVec3 objVel{ 100,10,0 };
	GSEVec3 objAcc{ 100,10,0 };
	float objMass = 1.0f;

	for (int i = 1; i < MAX_OBJECT_COUNT; ++i) {
		objPos.x = ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * 250.f;
		objPos.y = ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * 250.f;
		objSize.x = (float)std::rand() / (float)RAND_MAX * 5.f;

		objVel.x = ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * 3.f;
		objVel.y = ((float)std::rand() / (float)RAND_MAX - 0.5f) * 2.f * 3.f;
		objVel.z = 0.0f;

		m_objectMgr->AddObject(objPos, objSize, objVel, objAcc, objMass);
	}
}

void GSEGame::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	// Renderer Test
	for (int i = 0; i < MAX_OBJECT_COUNT; ++i) {
		GSEObject temp = m_objectMgr->GetObject(i);
		GSEVec3 pos = temp.GetPos();
		GSEVec3 size = temp.GetSize();
		if(size.x > std::numeric_limits<float>::min())m_renderer->DrawSolidRect(pos.x, pos.y, pos.z, size.x, 1, 0, 1, 1);
	}
}

void GSEGame::UpdateObjects(GSEKeyBoardMapper key_mapper, float elapsedTime)
{
	// add force
	float heroMovingForce = 10.0f;
	GSEVec3 heroForceDirection;
	if (key_mapper.W_key) heroForceDirection.y += 1.0f;
	if (key_mapper.A_key) heroForceDirection.x -= 1.0f;
	if (key_mapper.S_key) heroForceDirection.y -= 1.0f;
	if (key_mapper.D_key) heroForceDirection.x += 1.0f;
	
	heroForceDirection.x *= heroMovingForce;
	heroForceDirection.y *= heroMovingForce;
	heroForceDirection.z *= heroMovingForce;
	// update pos
	m_objectMgr->AddForce(m_hero_id, heroForceDirection, elapsedTime);

	if (m_objectMgr != NULL) m_objectMgr->UpdateObjects(elapsedTime);
}

GSEGame::~GSEGame()
{
	if (m_renderer != NULL) {
		delete m_renderer;
		m_renderer = NULL;
	}

	if (m_objectMgr != NULL) {
		delete m_objectMgr;
		m_objectMgr = NULL;
	}
}