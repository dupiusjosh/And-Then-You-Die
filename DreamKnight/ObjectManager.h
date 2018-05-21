#pragma once
#include <list>
#include "Object.h"
#include <mutex>
#include <unordered_map>
#include "QuadTree.h"
#include "Canvas.h"
#include "NavMesh.h"
class ObjectManager
{
	
	std::mutex m_dataLock;
	
	std::unordered_map<Object::ObjectTags, std::list<Object*> > m_TagMap;
	std::vector<Object*> m_StaticCollidableObjects;
	std::vector<Object*> m_NonStaticCollidableObjects; 
	std::vector<Object*> m_DynamicCollidableObjects;
	//std::vector<Object*> m_RenderableObjects;
	std::vector<Object*> m_ObjectsToAdd;
	//std::unordered_set<Object*> m_DontDestroyObjects;
	std::list<Object*> m_Objects;
	bool m_CompleteClear = false;
	bool m_ClearObjects = false;
	void SortAlive();
	void ActuallyAddObject(Object*o);
	void ActuallyClearObjects(bool complete = false);
	Canvas* m_Canvas = nullptr;
	Navigation::NavMesh * m_NavMesh;

public:
	static ObjectManager* GetInstance()
	{
		static ObjectManager* obj = new ObjectManager();
		return obj;
	}
	bool m_Dirty = true;

	QuadTree* m_StaticObjectQuadTree = nullptr;

	
	ObjectManager();
	~ObjectManager();
	void AddObject(Object* o);
	//void RemoveObject(Object* o);
	void Update(float deltaTime);
	
	Object* GetObjectByTag(Object::ObjectTags tag);

	std::vector<Object*>& GetStaticCollidableObjects();
	std::vector<Object*>& GetNonStaticCollidableObjects();
	std::vector<Object*>& GetDynamicCollidableObjects();

	std::vector<Object*> GetObjectsByTag(Object::ObjectTags tag);

	void BuildQuadTree();

	void SetNavMesh(Navigation::NavMesh * n) { m_NavMesh = n; }
	Navigation::NavMesh* GetNavMesh() { return m_NavMesh; }

	void ClearObjects(bool complete = false);
	//std::vector<Object*> GetObjectsByTag(Object::ObjectTags tag);

	std::vector<Object*> GetNearbyStaticObjects(Collider* collider);
	std::vector<Object*> GetStaticObjectsRaycast(glm::vec4 position, glm::vec4 direction);
	
	template<class T>
	std::vector<Object*> GetObjectsWithComponent();

	Canvas* GetCanvas()
	{
		if (m_Canvas == nullptr)
		{
			m_Canvas = new Canvas();
			AddObject(m_Canvas);
		}
		return m_Canvas;
	};
};


template<class T>
std::vector<Object*> ObjectManager::GetObjectsWithComponent()
{
	std::vector<Object*> objects;
	auto itr = m_Objects.begin();
	for (; itr != m_Objects.end(); ++itr)
	{
		if (!(*itr)->isEnabled()) break;
		if ((*itr)->GetComponent<T>() != NULL)
		{
			objects.push_back((*itr));
		}
	}
	return objects;
}