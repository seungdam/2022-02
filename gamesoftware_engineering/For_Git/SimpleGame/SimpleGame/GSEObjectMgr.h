#pragma once
#include "Global.h"
#include "GSEObject.h"
class GSEObjectMgr
{
private:
	// maximum 갯수의 오브젝트를 동적할당을 통해 관리.
	GSEObject* m_objects[MAX_OBJECT_COUNT];
public:
	GSEObjectMgr();
	~GSEObjectMgr();
	int AddObject(GSEVec3, GSEVec3, GSEVec3, GSEVec3, float);
	GSEObject GetObject(int);
	bool DeleteObject(int);
	int FindEmptySlot(); // 오브젝트 배열 중 빈 슬롯을 찾아 추가하게 한다.
	void UpdateObjects(float);
	void AddForce(int, GSEVec3, float);
};

