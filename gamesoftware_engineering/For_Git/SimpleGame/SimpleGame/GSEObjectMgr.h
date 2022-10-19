#pragma once
#include "Global.h"
#include "GSEObject.h"
class GSEObjectMgr
{
private:
	// maximum ������ ������Ʈ�� �����Ҵ��� ���� ����.
	GSEObject* m_objects[MAX_OBJECT_COUNT];
public:
	GSEObjectMgr();
	~GSEObjectMgr();
	int AddObject(GSEVec3, GSEVec3, GSEVec3, GSEVec3, float);
	GSEObject GetObject(int);
	bool DeleteObject(int);
	int FindEmptySlot(); // ������Ʈ �迭 �� �� ������ ã�� �߰��ϰ� �Ѵ�.
	void UpdateObjects(float);
	void AddForce(int, GSEVec3, float);
};

