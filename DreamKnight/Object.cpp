#include "Object.h"
#include "Collider.h"

#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "AnimationComponent.h"
#include "MeshCollider.h"

void Object::Update(float deltaTime)
{
	glm::mat4 prevTrans = m_Transform;
	for (Component* comp : m_Components)
	{
		if (comp->IsEnabled())
			comp->OnUpdate(deltaTime);
		bool NANI = false;
		for (unsigned int i = 0; i < 4; ++i)
		{
			for (unsigned int j = 0; j < 4; ++j)
			{
				if (isnan(m_Transform[i][j]))
				{
					//printf("TRANSY IS NANI?!\n");
					m_Transform = prevTrans;
					NANI = true;

					break;
				}
				if (isinf(m_Transform[i][j]))
				{
					//printf("TRANSY IS INF NANI?!\n");
					m_Transform = prevTrans;
					NANI = true;
					break;
				}
			}
			if (NANI) 
				break;
		}
		
	}

}

void Object::Start()
{
	for (size_t i=0; i<m_Components.size(); ++i)
		m_Components[i]->OnStart();
}

void Object::SetEnabled(bool val)
{
	m_ObjManager->m_Dirty = true;
	m_Enable = val;

}

bool Object::isEnabled() const
{
	if (m_Parent && !m_Parent->isEnabled())
		return false;
	return m_Enable;
}

bool Object::isPersistant() const
{
	if (m_Parent && m_Parent->isPersistant())
		return true;
	return m_Persistent;
}

void Object::AddComponent(Component * comp)
{
	//__uuidof(Object);
	//typeid(comp).name();
	//auto& inf = typeid(comp);
	//printf("%i %s\n", (int)inf.hash_code(), inf.name());
	if (dynamic_cast<Collider*>(comp) != NULL)
	{
		m_Colliders.push_back((Collider*)comp);
	}
	else if (dynamic_cast<MeshComponent*>(comp) != NULL)
	{
		m_MeshComp = (MeshComponent*)comp;
	}
	else if (dynamic_cast<MeshCollider*>(comp) != NULL)
	{
		m_MeshColliders.push_back((MeshCollider*)comp);
	}
	else if (dynamic_cast<MaterialComponent*>(comp) != NULL)
	{
		m_MaterialComp = (MaterialComponent*)comp;
	}
	else if (dynamic_cast<AnimationComponent*>(comp) != NULL)
	{
		m_AnimationComp = (AnimationComponent*)comp;
	}
	comp->m_GameObject = this;
	m_Components.push_back(comp);
}

void Object::SetParent(Object * parent)
{	
	if (m_Parent != parent)
	{
		if (m_Parent != nullptr)
			m_Parent->RemoveChild(this);
		m_Parent = parent;
		if (m_Parent != nullptr)
			m_Parent->AddChild(this);
	}
}

Object* Object::GetParent()
{
	return m_Parent;
}

void Object::AddChild(Object * child)
{
	m_Children.emplace(child);
	child->SetParent(this);
}

void Object::RemoveChild(Object * child)
{
	m_Children.erase(child);
	child->SetParent(this);
}

std::vector<Object*>& Object::GetChildren(std::vector<Object*>& vect)
{
	for (Object* obj : m_Children)
		vect.push_back(obj);
	return vect;
}

std::vector<Collider*>& Object::GetColliders()
{
	// TODO: insert return statement here
	return m_Colliders;
}

std::vector<MeshCollider*>& Object::GetMeshColliders()
{
	// TODO: insert return statement here
	return m_MeshColliders;
}

glm::mat4x4 Object::GetWorldMatrix()
{
	glm::mat4x4 mat = m_Transform;

	/*Object* currentParent = m_Parent;

	while (currentParent != nullptr)
	{
		mat = currentParent->m_Transform * mat;
		currentParent = currentParent->m_Parent;
	}*/
	if (m_Parent) {
		mat = m_Parent->GetWorldMatrix()*mat;
	}
	return mat;
}