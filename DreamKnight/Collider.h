#pragma once
#include <list>
#include <vector>
#include "CollidableComponent.h"
#include "ObjectManager.h"
//#include "MeshCollider.h"

//using namespace glm;

struct CollisionEvent;
class MeshCollider;

class Collider : public CollidableComponent
{
protected:
	bool m_isColliding;
	bool m_checkCollisionAgainstMesh = false;
	std::vector<Collider*> m_collidingWith;

	std::list<void(*)(CollisionEvent e)> m_onCollisionEnterEventList;
	std::list<void(*)(CollisionEvent e)> m_onCollisionStayEventList;
	std::list<void(*)(CollisionEvent e)> m_onCollisionExitEventList;

public:
	void NUKEIT();
	virtual void OnUpdate(float deltaTime);
	virtual bool CheckCollision(Collider& collider, CollisionEvent& collisionEvent) = 0;
	virtual void CheckCollisionAgainstCollider(Collider& collider);
	virtual void CheckCollisionAgainstMesh(MeshCollider& meshCollider) = 0;

	// Getters
	virtual const bool GetIsColliding();

	// Setters

	// Event Functions
	virtual void AddOnCollisionEnterEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront = false);
	virtual void AddOnCollisionStayEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront = false);
	virtual void AddOnCollisionExitEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront = false);
	virtual void ClearOnCollisionEnterEvents();
	virtual void ClearOnCollisionStayEvents();
	virtual void ClearOnCollisionExitEvents();
};

struct CollisionEvent
{
	CollisionEvent() {}

	CollisionEvent(CollidableComponent* thisObject, CollidableComponent* collidingWith)
	{
		m_ThisObject = thisObject;
		m_CollidingWith = collidingWith;
	};

	CollisionEvent(CollidableComponent* thisObject, CollidableComponent* collidingWith, glm::vec4 pointOfCollision, glm::vec4 collisionVector, float depth)
	{
		m_ThisObject = thisObject;
		m_CollidingWith = collidingWith;
		m_nCollisionVector = collisionVector;
		m_CollisionDepth = depth;
		m_PointOfCollision = pointOfCollision;
	};

	CollisionEvent(CollidableComponent* thisObject, CollidableComponent* collidingWith, glm::vec4 collisionVector, glm::vec3 faceNormal, float depth)
	{
		m_ThisObject = thisObject;
		m_CollidingWith = collidingWith;
		m_nCollisionVector = collisionVector;
		m_FaceNormal = faceNormal;
		m_CollisionDepth = depth;
	};

	CollisionEvent(CollidableComponent* thisObject, CollidableComponent* collidingWith, glm::vec4 collisionVector, glm::vec3 faceNormal, float depth, glm::vec4 pointOfCollision)
	{
		m_ThisObject = thisObject;
		m_CollidingWith = collidingWith;
		m_nCollisionVector = collisionVector;
		m_FaceNormal = faceNormal;
		m_CollisionDepth = depth;
		m_PointOfCollision = pointOfCollision;
	};

	CollidableComponent* m_ThisObject;
	CollidableComponent* m_CollidingWith;

	glm::vec4 m_nCollisionVector;
	glm::vec3 m_FaceNormal;
	float m_CollisionDepth;
	glm::vec4 m_PointOfCollision;
};

inline void Collider::NUKEIT()
{
	m_collidingWith.clear();
}

inline void Collider::OnUpdate(float deltaTime)
{
	////////////////////////////////////////////////////
	// Check all objects colliding on the previous frame
	////////////////////////////////////////////////////
	for (auto iter = m_collidingWith.begin(); iter != m_collidingWith.end() && m_collidingWith.size() != 0; )
	{
		auto collider = *iter;

		// Check if object is deleted or no longer colliding
		if ((*iter)->m_GameObject->isEnabled() == false || GetIsColliderEnabled() == false || collider->GetIsColliderEnabled() == false || CheckCollision(*collider, CollisionEvent(this, collider)) == false)
		{
			m_collidingWith.erase(iter);

			// Call OnCollisionExit events
			for (auto iter2 = m_onCollisionExitEventList.begin(); iter2 != m_onCollisionExitEventList.end(); ++iter2)
				(*iter2)(CollisionEvent(this, collider));

			// Workaround - Deleting the last element in a collection with an iterator causes a break when deleting the last element when inside a for loop
			iter = m_collidingWith.begin();
		}
		else
			++iter;
	}

	if (GetIsColliderEnabled() == false)
		return;

	/////////////////////////////////////////////////////
	// Collision for Static Objects
	// All Static Objects should exit within the QuadTree
	// QuadTree is queried by GetNearbyObjects()
	/////////////////////////////////////////////////////
	std::vector<Object*> staticObjects = m_GameObject->m_ObjManager->GetNearbyStaticObjects(this);
	size_t staticObjCount = staticObjects.size();

	for (size_t i = 0; i < staticObjCount; i++)
	{
		Object* obj = staticObjects[i];

		if (obj->isEnabled() == false)
			continue;

		std::vector<Collider*>& colliders = obj->GetColliders();
		std::vector<MeshCollider*>& meshColliders = obj->GetMeshColliders();
		size_t colliderCount = colliders.size();

		for (size_t j = 0; j < colliderCount; j++)
		{
			Collider * col = colliders[j];

			if (!col->GetIsColliderEnabled())
				continue;

			if (!m_GameObject->m_Static || !col->m_GameObject->m_Static)
				CheckCollisionAgainstCollider(*col);
		}

		colliderCount = meshColliders.size();

		for (size_t j = 0; j < colliderCount; j++)
		{
			if (m_checkCollisionAgainstMesh)
				CheckCollisionAgainstMesh(*meshColliders[j]); 
		}
	}

	///////////////////////////////////
	// Collision for non-static objects
	///////////////////////////////////
	std::vector<Object*> dynamicObjects = m_GameObject->m_ObjManager->GetDynamicCollidableObjects();
	size_t dynamicObjCount = dynamicObjects.size();

	for (size_t i = 0; i < dynamicObjCount; i++)
	{
		Object* obj = dynamicObjects[i];

		if (obj->isEnabled() == false)
			continue;

		std::vector<Collider*>& colliders = obj->GetColliders();
		std::vector<MeshCollider*>& meshColliders = obj->GetMeshColliders();
		size_t colliderCount = colliders.size();

		for (size_t j = 0; j < colliderCount; j++)
		{
			Collider* collider = colliders[j];

			if (!collider->GetIsColliderEnabled())
				continue;

			if (!m_GameObject->m_Static || !collider->m_GameObject->m_Static)
				CheckCollisionAgainstCollider(*collider);
		}

		colliderCount = meshColliders.size();

		for (size_t j = 0; j < colliderCount; j++)
		{
			if (m_checkCollisionAgainstMesh)
				CheckCollisionAgainstMesh(*meshColliders[j]);
		}
	}
}

inline void Collider::CheckCollisionAgainstCollider(Collider& collider)
{
	if (collider.m_GameObject == m_GameObject)
		return;

	CollisionEvent collisionEvent(this, &collider);

	// Collision Happens
	if (collider.m_GameObject->isEnabled() == true && CheckCollision(collider, collisionEvent))
	{
		bool collisionStay = false;

		// Check if object was colliding with this object on the previous frame
		for (unsigned int j = 0; j < m_collidingWith.size(); j++)
		{
			if (&collider == m_collidingWith[j])
				collisionStay = true;
		}

		// If object was collidion on pevious frame call OnCollisionStay events
		if (collisionStay)
		{
			for (auto iter = m_onCollisionStayEventList.begin(); iter != m_onCollisionStayEventList.end(); ++iter)
				(*iter)(collisionEvent);
		}
		// Otherwise call OnCollisionEnter events
		else
		{
			for (auto iter = m_onCollisionEnterEventList.begin(); iter != m_onCollisionEnterEventList.end(); ++iter)
			{
				m_collidingWith.push_back(&collider);
				(*iter)(collisionEvent);
			}
		}
	}
}

inline const bool Collider::GetIsColliding()
{
	bool isColliding;

	//m_Mutex.lock();
	isColliding = m_isColliding;
	//m_Mutex.unlock();

	return isColliding;
}

inline void Collider::AddOnCollisionEnterEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront)
{
	if (pushFront)
		m_onCollisionEnterEventList.push_front(collisionEvent);
	else
		m_onCollisionEnterEventList.push_back(collisionEvent);
}

inline void Collider::AddOnCollisionStayEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront)
{
	if (pushFront)
		m_onCollisionStayEventList.push_front(collisionEvent);
	else
		m_onCollisionStayEventList.push_back(collisionEvent);
}

inline void Collider::AddOnCollisionExitEvent(void(*collisionEvent)(CollisionEvent e), bool pushFront)
{
	if (pushFront)
		m_onCollisionExitEventList.push_front(collisionEvent);
	else
		m_onCollisionExitEventList.push_back(collisionEvent);
}

inline void Collider::ClearOnCollisionEnterEvents()
{
	m_onCollisionEnterEventList.clear();
}

inline void Collider::ClearOnCollisionStayEvents()
{
	m_onCollisionStayEventList.clear();
}

inline void Collider::ClearOnCollisionExitEvents()
{
	m_onCollisionExitEventList.clear();
}