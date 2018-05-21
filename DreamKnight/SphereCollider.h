#pragma once
#include "Collider.h"

class MeshCollider;

class SphereCollider : public Collider
{
private:
	glm::vec4 m_position;
	float m_radius;

public:
	SphereCollider();
	SphereCollider(glm::vec4 position, float radius, ColliderTag tag, bool isTrigger = true, bool checkCollisionAgainstMesh = false);

	bool CheckCollision(Collider& collider, CollisionEvent& collisionEvent) override;
	void CheckCollisionAgainstMesh(MeshCollider& meshCollider) override;

	// Getters
	const glm::vec4 GetPosition();
	const glm::vec4 GetWorldPosition();
	const float GetRadius();

	// Setters
	void SetPosition(glm::vec4 position);
	void SetRadius(float radius);

	~SphereCollider();
};