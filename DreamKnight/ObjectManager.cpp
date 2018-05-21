#include "ObjectManager.h"
#include "AABBCollider.h"
#include "MeshCollider.h"

void ObjectManager::SortAlive()
{
	/*std::list<Object*>::iterator itr = m_Objects.begin();
	for (; itr != m_Objects.end(); ++itr)
	{
		if ((*itr)->m_Enable)
		{
			m_Objects.push_front((*itr));
		}
		else
		{
			m_Objects.push_back((*itr));
		}
		m_Objects.erase(itr);
	}*/
	if (m_Dirty)
	{
		m_Objects.sort([](const Object* a, const Object * b)
		{
			return (a->isEnabled() > b->isEnabled());
		});
		m_Dirty = false;
	}
}

void ObjectManager::ActuallyAddObject(Object * o)
{
	o->m_ObjManager = this;

	if ((o->GetColliders().size() > 0 && o->m_Static) || (o->GetMeshColliders().size() > 0 && o->m_Static))
	{
		m_StaticCollidableObjects.push_back(o);
	}
	else
	{
		m_NonStaticCollidableObjects.push_back(o);
	}
	if ((o->GetMeshColliders().size() > 0 && !o->m_Static) || (o->GetColliders().size() > 0 && !o->m_Static))
	{
		m_DynamicCollidableObjects.push_back(o);
	}

	m_Objects.push_back(o);
	o->Start();

	m_Dirty = true;
}

void ObjectManager::ActuallyClearObjects(bool complete)
{
	std::list<Object*> persistObj;
	m_dataLock.lock();
	m_StaticCollidableObjects.clear();
	m_NonStaticCollidableObjects.clear();
	m_DynamicCollidableObjects.clear();
	//m_RenderableObjects.clear();

	//Delete objects to add
	for (auto iter = m_ObjectsToAdd.begin(); iter != m_ObjectsToAdd.end(); ++iter)
	{
		if (m_Canvas == (*iter))
			m_Canvas = nullptr;
		delete (*iter);
		(*iter) = nullptr;
	}
	m_ObjectsToAdd.clear();



	//m_ObjectsToAdd = m_DontDestroyObjects;
	auto iter = m_Objects.begin();
	for (; iter != m_Objects.end(); ++iter)
	{


		if (m_Canvas == (*iter))
			m_Canvas = nullptr;
		//Should work, shouldn't check persistance if complete is checked first.
		if (complete || !(*iter)->isPersistant())
		{
			delete (*iter);
			(*iter) = nullptr;
		}
		else
		{
			//printf("%d %f %f %f\n", (*iter)->tag, (*iter)->m_Transform[3][0], (*iter)->m_Transform[3][1], (*iter)->m_Transform[3][2]);
			m_ObjectsToAdd.push_back((*iter));
		}
	}
	//Not sure if canvas is in obj manager...
	m_Objects.clear();
	//m_ObjectsToAdd = persistObj;
	for (unsigned int i = 0; i < m_ObjectsToAdd.size(); ++i)
	{
		Object* o = m_ObjectsToAdd[i];
		o->m_ObjManager = this;
		auto & shtuff = o->GetColliders();
		for (unsigned int i = 0; i < shtuff.size(); ++i)
		{
			shtuff[i]->NUKEIT();
		}
		if ((o->GetColliders().size() > 0 && o->m_Static) || (o->GetMeshColliders().size() > 0 && o->m_Static))
		{
			m_StaticCollidableObjects.push_back(o);
		}
		else
		{
			m_NonStaticCollidableObjects.push_back(o);
		}
		if ((o->GetMeshColliders().size() > 0 && !o->m_Static) || (o->GetColliders().size() > 0 && !o->m_Static))
		{
			m_DynamicCollidableObjects.push_back(o);
		}
		//printf("%d %f %f %f\n", (o)->tag, (o)->m_Transform[3][0], (o)->m_Transform[3][1], (o)->m_Transform[3][2]);

		m_Objects.push_back(o);
		
	}
	m_ObjectsToAdd.clear();
	m_ClearObjects = false;
	delete m_StaticObjectQuadTree;
	m_StaticObjectQuadTree = nullptr;
	m_dataLock.unlock();
	m_CompleteClear = false;
	m_Dirty = true;
}

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
	ActuallyClearObjects(true);

	delete m_StaticObjectQuadTree;
}

void ObjectManager::AddObject(Object * o)
{
	m_ObjectsToAdd.push_back(o);
	if (Canvas* canvas = dynamic_cast<Canvas*>(o))
		m_Canvas = canvas;
}

void ObjectManager::Update(float deltaTime)
{

	if (m_ClearObjects)
		ActuallyClearObjects(m_CompleteClear);
	
	m_dataLock.lock();
	if (deltaTime > FLT_EPSILON)
	{
		if (deltaTime > 1.0f / 30.0f)
			deltaTime = 1.0f / 30.0f;
		//printf("Updating objects count: %d\n", m_NonStaticCollidableObjects.size());
		auto itr = m_Objects.begin();
		for (; itr != m_Objects.end(); ++itr)
		{
			if (!(*itr)->isEnabled())  break;
			(*itr)->Update(deltaTime);
		}
	}
	for (unsigned int i = 0; i < m_ObjectsToAdd.size(); ++i)
	{
		ActuallyAddObject(m_ObjectsToAdd[i]);
	}
	m_ObjectsToAdd.clear();


	SortAlive();
	m_dataLock.unlock();

}

Object * ObjectManager::GetObjectByTag(Object::ObjectTags tag)
{
	Object* obj;
	auto itr = m_Objects.begin();
	for (; itr != m_Objects.end(); ++itr)
	{
		if (!(*itr)->isEnabled())  break;
		if ((*itr)->tag == tag) { obj = (*itr); break; }
	}
	return obj;
}

std::vector<Object*>& ObjectManager::GetStaticCollidableObjects()
{
	return m_StaticCollidableObjects;
}

std::vector<Object*>& ObjectManager::GetNonStaticCollidableObjects()
{
	return m_NonStaticCollidableObjects;
}

std::vector<Object*>& ObjectManager::GetDynamicCollidableObjects()
{
	return m_DynamicCollidableObjects;
}


std::vector<Object*> ObjectManager::GetObjectsByTag(Object::ObjectTags tag)
{
	std::vector<Object*> objects;
	auto itr = m_Objects.begin();
	for (; itr != m_Objects.end(); ++itr)
	{
		if (!(*itr)->isEnabled())  break;
		if ((*itr)->tag == tag)
		{
			objects.push_back((*itr));
		}
	}
	return objects;
}

std::vector<Object*> ObjectManager::GetNearbyStaticObjects(Collider * collider)
{
	if (!m_StaticObjectQuadTree)
		return std::vector<Object*>();
	return m_StaticObjectQuadTree->Retrieve(*collider);
}

std::vector<Object*> ObjectManager::GetStaticObjectsRaycast(glm::vec4 position, glm::vec4 direction)
{
	if (!m_StaticObjectQuadTree)
		return std::vector<Object*>();
	return m_StaticObjectQuadTree->Retrieve(position, direction);
}

void ObjectManager::ClearObjects(bool complete)
{
	m_CompleteClear = complete;
	m_ClearObjects = true;
}

void ObjectManager::BuildQuadTree()
{
	glm::vec4 min = { FLT_MAX, FLT_MAX, FLT_MAX, 1.0f };
	glm::vec4 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f };
	std::vector<Object*> objs = GetStaticCollidableObjects();

	for (size_t i = 0; i < objs.size(); i++)
	{
		auto colliders = objs[i]->GetMeshColliders();

		if (colliders.size() > 0 && objs[i]->m_Static)
		{
			auto bounds = &(colliders[0])->GetBounds();
			bounds->m_GameObject = objs[i];

			if (bounds->GetWorldMax().x > max.x)
				max.x = bounds->GetWorldMax().x;
			if (bounds->GetWorldMax().y > max.y)
				max.y = bounds->GetWorldMax().y;
			if (bounds->GetWorldMax().z > max.z)
				max.z = bounds->GetWorldMax().z;

			if (bounds->GetWorldMin().x < min.x)
				min.x = bounds->GetWorldMin().x;
			if (bounds->GetWorldMin().y < min.y)
				min.y = bounds->GetWorldMin().y;
			if (bounds->GetWorldMin().z < min.z)
				min.z = bounds->GetWorldMin().z;
		}
	}

	QuadTree* tree = new QuadTree(0, 25, new AABBCollider(min, max, ColliderTag::Other));

	std::vector<Object*>& objects = GetStaticCollidableObjects();

	for (size_t i = 0; i < objects.size(); i++)
	{
		auto meshCol = objects[i]->GetMeshColliders();

		if (meshCol.size() > 0)
			tree->Insert(objects[i]);
	}

	m_StaticObjectQuadTree = tree;
}
