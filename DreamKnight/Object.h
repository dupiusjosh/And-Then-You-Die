#pragma once
#include <vector>
#include "Component.h"
#include "ObjectType.h"
#include "glm.h"
#include <typeinfo>
#include <unordered_set>

//struct CollidableComponenet;
class Collider;
class MeshCollider;
class AnimationComponent;
struct MeshComponent;
struct MaterialComponent;
class ObjectManager;
class Object
{
protected:
	bool m_Enable = true;

	std::vector<Component*> m_Components;
	std::vector<Collider*> m_Colliders;
	std::vector<MeshCollider*> m_MeshColliders;
	
	Object* m_Parent = nullptr;
	std::unordered_set<Object*> m_Children;

public:
	//If a type is added for networking, must add to NetDefines create object function

	unsigned long* m_MyHash= nullptr;
	unsigned long m_Creator=0;
	enum ObjectTags
	{
		DEFAULT=-1,
		Player = 0,
		Enemy = 1,
		Wall = 2,
		Floor = 3,
		World = 4,
		Pickup = 5,
		Chest = 6,
		TagCount
	};

	ObjectTags tag = DEFAULT;

	virtual ~Object()
	{
		for (unsigned int i = 0; i < m_Components.size(); ++i)
		{
			delete m_Components[i];
			//m_Components[i] = nullptr;
		}

	}

	ObjectManager * m_ObjManager = nullptr;
	glm::mat4 m_Transform = glm::mat4(1);
	bool m_Static = true;
	bool m_Persistent = false;
	MeshComponent * m_MeshComp = nullptr;
	MaterialComponent * m_MaterialComp = nullptr;
	AnimationComponent * m_AnimationComp = nullptr;


	ObjectType m_ObjectType;

	
	virtual void Update(float deltaTime);
	virtual void Start();
	void SetEnabled(bool val);
	bool isEnabled() const;
	bool isPersistant() const;
	void AddComponent(Component* comp);
	
	void SetParent(Object* parent);
	Object * GetParent();
	void AddChild(Object* parent);
	void RemoveChild(Object* parent);
	std::vector<Object*>& Object::GetChildren(std::vector<Object*>& vect);

	std::vector<Collider*>& GetColliders();
	std::vector<MeshCollider*>& GetMeshColliders();

	template<class t>
	t* GetComponent();

	template<class t>
	std::vector<t*> GetAllComponents();
	glm::mat4x4 GetWorldMatrix();
};

template<class t>
t* Object::GetComponent()
{
	t * obj= NULL;
	for (unsigned int i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->IsEnabled())
		{
			obj = dynamic_cast<t*>(m_Components[i]);
			if (obj) break;
		}
	}
	return obj;
}

template<class t>
std::vector<t*> Object::GetAllComponents()
{
	std::vector<t*> retVec;
	t * obj = NULL;
	for (unsigned int i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->IsEnabled())
		{
			obj = dynamic_cast<t*>(m_Components[i]);
			if (obj)
				retVec.push_back(obj);
		}
	}
	retVec.shrink_to_fit();
	return retVec;
}