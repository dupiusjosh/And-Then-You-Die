#pragma once
#include "Collider.h"

struct Mesh;
class MeshCollider;

class CapsuleCollider : public Collider
{
private:
	float m_radius;
	glm::vec4 m_start;
	glm::vec4 m_end;

public:
	CapsuleCollider();
	CapsuleCollider(glm::vec4 start, glm::vec4 end, float radius, ColliderTag tag, bool isTrigger = true, bool checkCollisionAgainstMesh = false);

	bool CheckCollision(Collider& collider, CollisionEvent& collisionEvent) override;
	void CheckCollisionAgainstMesh2(MeshCollider& meshCollider);
	void CheckCollisionAgainstMesh(MeshCollider& meshCollider) override;

	// Getters
	const glm::vec4 GetStart();
	const glm::vec4 GetWorldStart();
	const glm::vec4 GetEnd();
	const glm::vec4 GetWorldEnd();
	const float GetRadius();

	// Setters
	void SetRadius(float radius);
	void SetStart(glm::vec4 start);
	void SetEnd(glm::vec4 end);

	void BuildCapsuleAroundMesh();

	~CapsuleCollider();
};