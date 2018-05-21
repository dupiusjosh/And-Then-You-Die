#pragma once
#include"Collider.h"

class MeshCollider;

class AABBCollider : public Collider
{
private:
	glm::vec4 m_min;
	glm::vec4 m_max;

public:
	AABBCollider();
	AABBCollider(glm::vec4 min, glm::vec4 max, ColliderTag tag, bool isTrigger = true, bool checkCollisionAgainstMesh = false);

	bool CheckCollision(Collider& collider, CollisionEvent& collisionEvent) override;

	// TODO: Implement AABB to Mesh
	void CheckCollisionAgainstMesh(MeshCollider& meshCollider) override;

	// Getters
	const glm::vec4 GetMin();
	const glm::vec4 GetWorldMin();
	const glm::vec4 GetMax();
	const glm::vec4 GetWorldMax();
	glm::vec4 GetCenter() const;

	void DrawLines(glm::vec4 color);

	// Setters
	void SetMin(glm::vec4 min);
	void SetMax(glm::vec4 max);

	~AABBCollider();
};