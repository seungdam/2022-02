#pragma once
#include "Global.h"
class GSEObject
{
	GSEVec3 m_pos;
	GSEVec3 m_size;
	GSEVec3 m_vel;
	GSEVec3 m_acc;
	float m_mass;
public:
	GSEObject() {};
	GSEObject(GSEVec3, GSEVec3, GSEVec3, GSEVec3, float);
	GSEVec3 GetPos() { return m_pos; };
	void SetPos(GSEVec3);
	GSEVec3 GetSize() { return m_size; };
	void SetSize(GSEVec3);
	GSEVec3 GetVel() { return m_vel; };
	void SetVel(GSEVec3);
	GSEVec3 GetAcc() { return m_acc; };
	void SetAcc(GSEVec3);
	float GetMass()  { return m_mass; };
	void SetMass(float);
	void AddForce(GSEVec3, float);
	void Update(float);
	~GSEObject();
};

