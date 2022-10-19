#include "stdafx.h"
#include "GSEObjectMgr.h"
#include <iostream>

GSEObjectMgr::GSEObjectMgr()
{
	// 해당 오브젝트 객체를 NULL로 초기화
	for (auto& i : m_objects) {
		i = NULL;
	}
}

GSEObjectMgr::~GSEObjectMgr()
{
	for (int i = 0; i < MAX_OBJECT_COUNT; ++i) {
		DeleteObject(i);
	}
}

int GSEObjectMgr::AddObject(GSEVec3 pos, GSEVec3 size, GSEVec3 vel, GSEVec3 acc, float mass)
{
	// find empty slot
	int idx = FindEmptySlot();
	if (idx < 0) {
		std::cout << "No More Empty Slot" << std::endl;
		return -1; // all the slot is full
	}
	m_objects[idx] = new GSEObject(pos, size, vel, acc,mass);
	return idx;
}

GSEObject GSEObjectMgr::GetObject(int idx)
{
	GSEVec3 pos{0,0,0};
	GSEVec3 size{ 0,0,0 };
	GSEVec3 vel{ 0,0,0 };
	GSEVec3 acc{ 0,0,0 };
	float mass = 0;
	GSEObject temp = GSEObject(pos, size, vel, acc, mass);
	if (m_objects[idx] != NULL) memcpy(&temp, m_objects[idx], sizeof(GSEObject));
	else {}// log  출력 
	return temp;
}

bool GSEObjectMgr::DeleteObject(int idx)
{
	if (m_objects[idx] != NULL) {
		delete m_objects[idx];
		m_objects[idx] = NULL;
		return true;
	}
	return false;
}

int GSEObjectMgr::FindEmptySlot()
{
	int idx = 0;
	for (auto i : m_objects) {
		if (i == NULL) return idx;// is empty slot?
		++idx;
	}
	return -1;
}

void GSEObjectMgr::UpdateObjects(float elapsedTime)
{
	for (auto& i : m_objects) {
		if (i != NULL) i->Update(elapsedTime);
	}
}

void GSEObjectMgr::AddForce(int idx, GSEVec3 force, float eTime)
{
	if(m_objects[idx] != NULL) m_objects[idx]->AddForce(force, eTime);
}
