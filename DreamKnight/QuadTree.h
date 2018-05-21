#pragma once
#include <vector>
#include "glm.h"
//#include "Frustum.h"

//using namespace glm;

class Object;
class Collider;
class AABBCollider;
class Frustum;

class QuadTree
{
private:
	int m_level;
	unsigned m_maxObjects;
	AABBCollider* m_bounds;

	std::vector<Object*> m_objects;
	std::vector<QuadTree*> m_nodes;

public:
	QuadTree();
	QuadTree(int level, unsigned int maxObjects, AABBCollider* bounds);

	const AABBCollider* GetBounds() const;

	void Split();
	bool Insert(Object* object);
	std::vector<Object*> Retrieve(Collider& collider);
	void Retrieve(Collider * collider, std::vector<Object*>* objects);
	void AddAllChildren(std::vector<Object*>* objects);
	std::vector<Object*> Retrieve(Frustum * frustum);
	void Retrieve(Frustum * frustum, std::vector<Object*>* objects);
	std::vector<Object*> Retrieve(glm::vec4 position, glm::vec4 direction);
	void Retrieve(glm::vec4 position, glm::vec4 direction, std::vector<Object*>* objects);
	std::vector<QuadTree*> RetrieveNodes(glm::vec4 position, glm::vec4 direction);
	void Retrieve(glm::vec4 position, glm::vec4 direction, std::vector<QuadTree*>* objects);
	Object * Retrieve(glm::vec4 position, glm::vec4 direction, glm::vec4& hit);
	//std::vector<Object*> Retrieve(glm::vec4 position, glm::vec4 direction);

	~QuadTree();
	void DrawLines();
};