#pragma once
#include "Object.h"

class Projectile : public Object
{
private:
	Object* m_Owner;
	bool m_IsPlayers;
	float m_Damage;
public:
	Projectile();
	Projectile(bool _IsPlayers);
	~Projectile();
	void SetIsPlayer(bool _setter) { m_IsPlayers = _setter; }
	void SetDamage(float damage);
	float GetDamage();
	//void HitCheck(CollisionEvent e);
	Object* GetOwner();
	void	SetOwner(Object* owner);
};