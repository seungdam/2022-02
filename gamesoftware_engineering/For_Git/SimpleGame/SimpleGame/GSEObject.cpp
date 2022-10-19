#include "stdafx.h"
#include "GSEObject.h"

GSEObject::GSEObject(GSEVec3 pos, GSEVec3 size, GSEVec3 vel, GSEVec3 acc, float mass):m_pos(pos),m_size(size),m_vel(vel),m_acc(acc),m_mass(mass)
{
}

void GSEObject::SetPos(GSEVec3 in)
{
	m_pos = in;
}

void GSEObject::SetSize(GSEVec3 in)
{
	m_size = in;
}

void GSEObject::SetVel(GSEVec3 in)
{
	m_vel = in;
}

void GSEObject::SetAcc(GSEVec3 in)
{
	m_acc = in;
}

void GSEObject::SetMass(float in)
{
	m_mass = in;
}

void GSEObject::AddForce(GSEVec3 force, float eTime)
{
	GSEVec3 acc;
	acc.x = force.x / m_mass;
	acc.y = force.y / m_mass;
	acc.z = force.z / m_mass;
	m_vel.x += acc.x * eTime;
	m_vel.y += acc.y * eTime;
	m_vel.z += acc.z * eTime;
}

void GSEObject::Update(float elapsedTime)
{
	m_pos.x += (m_vel.x * elapsedTime);
	m_pos.y += (m_vel.y * elapsedTime);
	m_pos.z += (m_vel.z * elapsedTime);
}

GSEObject::~GSEObject()
{
}
