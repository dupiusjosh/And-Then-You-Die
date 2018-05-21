#include "QuadTree.h"
#include "Object.h"
#include "MeshCollider.h"
#include "CollisionDetection.h"
#include "Frustum.h"
#include "Renderer.h"
#include <algorithm>

QuadTree::QuadTree()
{
}

QuadTree::QuadTree(int level, unsigned int maxObjects, AABBCollider* bounds)
{
	m_level = level;
	m_maxObjects = maxObjects;
	m_bounds = bounds;
}

const AABBCollider* QuadTree::GetBounds() const
{
	return m_bounds;
}

void QuadTree::Split()
{
	float subWidth = 0.5f * (m_bounds->GetMax().x - m_bounds->GetMin().x);
	float subDepth = 0.5f * (m_bounds->GetMax().z - m_bounds->GetMin().z);

	/// Q1-Q2
	/// Q3-Q4
	glm::vec4 q1Min, q2Min, q3Min, q4Min;
	glm::vec4 q1Max, q2Max, q3Max, q4Max;

	q1Max = { m_bounds->GetMax().x - subWidth, m_bounds->GetMax().y, m_bounds->GetMax().z, 1.0f };
	q1Min = { m_bounds->GetMin().x, m_bounds->GetMin().y, m_bounds->GetMin().z + subDepth, 1.0f };

	q2Max = { m_bounds->GetMax().x, m_bounds->GetMax().y, m_bounds->GetMax().z, 1.0f };
	q2Min = { m_bounds->GetMax().x - subWidth, m_bounds->GetMin().y, m_bounds->GetMin().z + subDepth, 1.0f };

	q3Max = { m_bounds->GetMax().x - subWidth, m_bounds->GetMax().y, m_bounds->GetMin().z + subDepth, 1.0f };
	q3Min = { m_bounds->GetMin().x , m_bounds->GetMin().y, m_bounds->GetMin().z, 1.0f };

	q4Max = { m_bounds->GetMax().x, m_bounds->GetMax().y, m_bounds->GetMax().z - subDepth, 1.0f };
	q4Min = { m_bounds->GetMin().x + subWidth, m_bounds->GetMin().y, m_bounds->GetMin().z, 1.0f };

	m_nodes.push_back(new QuadTree(m_level + 1, m_maxObjects, new AABBCollider(q1Min, q1Max, ColliderTag::Other)));
	m_nodes.push_back(new QuadTree(m_level + 1, m_maxObjects, new AABBCollider(q2Min, q2Max, ColliderTag::Other)));
	m_nodes.push_back(new QuadTree(m_level + 1, m_maxObjects, new AABBCollider(q3Min, q3Max, ColliderTag::Other)));
	m_nodes.push_back(new QuadTree(m_level + 1, m_maxObjects, new AABBCollider(q4Min, q4Max, ColliderTag::Other)));

	std::vector<Object*> objectsNotAdded;

	// Move objects into new children
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		bool added = false;
		Collider* objectBounds = &(m_objects[i]->GetMeshColliders()[0]->GetBounds());

		if (m_nodes[0]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		{
			m_nodes[0]->m_objects.push_back(objectBounds->m_GameObject);
			added = true;
		}

		if (m_nodes[1]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		{
			m_nodes[1]->m_objects.push_back(objectBounds->m_GameObject);
			added = true;
		}

		if (m_nodes[2]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		{
			m_nodes[2]->m_objects.push_back(objectBounds->m_GameObject);
			added = true;
		}

		if (m_nodes[3]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		{
			m_nodes[3]->m_objects.push_back(objectBounds->m_GameObject);
			added = true;
		}

		if (!added)
			objectsNotAdded.push_back(objectBounds->m_GameObject);
	}

	// Clear out parent container
	m_objects.clear();

	// Re-add objects that were not within any of the child bounds
	// This should never happen
	for (size_t i = 0; i < objectsNotAdded.size(); i++)
		m_objects.push_back(objectsNotAdded[i]);
}

bool QuadTree::Insert(Object* object)
{
	AABBCollider* objectBounds = &(object->GetMeshColliders()[0]->GetBounds());
	objectBounds->m_GameObject = object;

	if (!m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		return false;

	if (m_objects.size() < m_maxObjects && m_nodes.size() == 0)
	{
		m_objects.push_back(object);
		return true;
	}

	if (m_nodes.size() == 0)
		Split();

	bool inserted = false;

	if (m_nodes[0]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		inserted = m_nodes[0]->Insert(object);
	//m_nodes[0]->m_objects.push_back(collider->m_GameObject);

	if (m_nodes[1]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		inserted = m_nodes[1]->Insert(object);
	//m_nodes[1]->m_objects.push_back(collider->m_GameObject);

	if (m_nodes[2]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		inserted = m_nodes[2]->Insert(object);
	//m_nodes[2]->m_objects.push_back(collider->m_GameObject);

	if (m_nodes[3]->m_bounds->CheckCollision(*objectBounds, CollisionEvent()))
		inserted = m_nodes[3]->Insert(object);
	//m_nodes[3]->m_objects.push_back(collider->m_GameObject);

	return inserted;
}

std::vector<Object*> QuadTree::Retrieve(Collider& collider)
{
	std::vector<Object*> objects;
	Retrieve(&collider, &objects);
	return objects;
}

void QuadTree::Retrieve(Collider* collider, std::vector<Object*>* objects)
{
	if (!m_bounds->CheckCollision(*collider, CollisionEvent()))
		return;

	for (Object* obj : m_objects)
		objects->push_back(obj);

	for (size_t i = 0; i<m_nodes.size(); ++i)
		m_nodes[i]->Retrieve(collider, objects);
}

void QuadTree::AddAllChildren(std::vector<Object*>* objects)
{
	for (Object* obj : m_objects)
		objects->push_back(obj);

	for (size_t i = 0; i < m_nodes.size(); ++i)
		m_nodes[i]->AddAllChildren(objects);
}

std::vector<Object*> QuadTree::Retrieve(Frustum* frustum)
{
	std::vector<Object*> objects;
	Retrieve(frustum, &objects);
	return objects;
}

void QuadTree::Retrieve(Frustum* frustum, std::vector<Object*>* objects)
{
	int inside = frustum->FrustumToAABB(*m_bounds);
	if (inside == 1)
	{
		//m_bounds->DrawLines(glm::vec4(1, 0, 0, 1));
		return;
	}

	for (Object* obj : m_objects)
		objects->push_back(obj);

	if (inside == -1)
	{
		AddAllChildren(objects);
		//m_bounds->DrawLines(glm::vec4(0, 1, 0, 1));
	}

	if (inside == 0)
	{
		if (m_nodes.size() == 0)
		{
			//m_bounds->DrawLines(glm::vec4(1, 1, 0, 1));
			return;
		}

		for (size_t i = 0; i<m_nodes.size(); ++i)
			m_nodes[i]->Retrieve(frustum, objects);
	}
}

std::vector<Object*> QuadTree::Retrieve(glm::vec4 position, glm::vec4 direction)
{
	std::vector<Object*> objects;
	Retrieve(position, direction, &objects);
	return objects;
}

void QuadTree::Retrieve(glm::vec4 position, glm::vec4 direction, std::vector<Object*>* objects)
{
	if (CollisionDetection::RaycastToAABB(position, direction, m_bounds) > 0)
	{
		for (size_t i = 0; i < m_objects.size(); i++)
			objects->push_back(m_objects[i]);

		if (m_nodes.size() == 0)
			return;

		m_nodes[0]->Retrieve(position, direction, objects);
		m_nodes[1]->Retrieve(position, direction, objects);
		m_nodes[2]->Retrieve(position, direction, objects);
		m_nodes[3]->Retrieve(position, direction, objects);
	}
}

std::vector<QuadTree*> QuadTree::RetrieveNodes(glm::vec4 position, glm::vec4 direction)
{
	std::vector<QuadTree*> nodes;
	Retrieve(position, direction, &nodes);
	return nodes;
}

void QuadTree::Retrieve(glm::vec4 position, glm::vec4 direction, std::vector<QuadTree*>* nodes)
{
	if (CollisionDetection::RaycastToAABB(position, direction, m_bounds) > 0)
	{
		if (m_nodes.size() == 0)
		{
			nodes->push_back(this);
			//DrawLines();
			return;
		}
		m_nodes[0]->Retrieve(position, direction, nodes);
		m_nodes[1]->Retrieve(position, direction, nodes);
		m_nodes[2]->Retrieve(position, direction, nodes);
		m_nodes[3]->Retrieve(position, direction, nodes);
	}
}

Object* QuadTree::Retrieve(glm::vec4 position, glm::vec4 direction, glm::vec4& hit)
{
	//Renderer::GetInstance()->AddLine(position, position + direction * 100.0f);
	std::vector<QuadTree*> nodes;
	Retrieve(position, direction, &nodes);

	std::sort(nodes.begin(), nodes.end(), [position](const QuadTree* a, const QuadTree* b) -> bool
	{
		float dist1 = glm::length2(a->GetBounds()->GetCenter() - position);
		float dist2 = glm::length2(b->GetBounds()->GetCenter() - position);
		return dist1 < dist2;
	});

	for (size_t i = 0; i < nodes.size(); ++i)
	{
		Object* obj = CollisionDetection::RaycastAgainstClosest(position, direction, nodes[i]->m_objects);
		if (obj)
		{
			obj->GetMeshColliders()[0]->GetBounds().DrawLines(glm::vec4(1, 1, 0, 1));
			hit = CollisionDetection::RaycastToAABBPoint(position, direction, &obj->GetMeshColliders()[0]->GetBounds());
			return obj;
		}
	}

	return nullptr;
}

QuadTree::~QuadTree()
{
	delete m_bounds;
	m_bounds = nullptr;

	for (size_t i = 0; i < m_nodes.size(); i++)
	{
		delete m_nodes[i];
		m_nodes[i] = nullptr;
	}
}

void QuadTree::DrawLines()
{
	m_bounds->DrawLines(glm::vec4(1));
	for (auto node : m_nodes)
		node->DrawLines();
}