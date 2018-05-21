#pragma once
#include <mutex>
#include "glm.h"
#include "Object.h"
#include "Component.h"
#include "Mesh.h"

//using namespace glm;

enum class ColliderType
{
	AABB,
	Capsule,
	Plane,
	Sphere,
	Mesh,
	Frustum
};

enum class ColliderTag
{
	Unit,
	HitBox,
	HurtBox,
	Other
};

class CollidableComponent : public Component
{
protected:
	ColliderType m_colliderType;
	ColliderTag m_colliderTag;
	std::mutex m_Mutex;
	bool m_isTrigger;
	bool m_isColliderEnabled = true;

public:
	const ColliderType GetColliderType();
	bool GetIsColliderEnabled();
	virtual const ColliderTag GetColliderTag();
	virtual const bool GetIsTrigger();

	void SetEnabled(bool enabled) override;
	virtual const void SetColliderTag(ColliderTag tag);
	virtual const void SetIsTrigger(bool isTrigger);
};

inline const ColliderType CollidableComponent::GetColliderType()
{
	ColliderType type;

	//m_Mutex.lock();
	type = m_colliderType;
	//m_Mutex.unlock();

	return type;
}

inline void CollidableComponent::SetEnabled(bool enabled)
{
	m_isColliderEnabled = enabled;
}

inline bool CollidableComponent::GetIsColliderEnabled()
{
	return m_isColliderEnabled;
}

inline const ColliderTag CollidableComponent::GetColliderTag()
{
	return m_colliderTag;
}

inline const bool CollidableComponent::GetIsTrigger()
{
	return m_isTrigger;
}

inline const void CollidableComponent::SetColliderTag(ColliderTag tag)
{
	m_colliderTag = tag;
}

inline const void CollidableComponent::SetIsTrigger(bool isTrigger)
{
	m_isTrigger = isTrigger;
}
