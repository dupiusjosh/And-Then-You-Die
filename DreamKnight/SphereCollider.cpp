#include "SphereCollider.h"
#include "MeshCollider.h"
#include "CollisionDetection.h"
//#include <glm\simd\matrix.h>
//#include <glm\gtx\simd_mat4.hpp>
//#include <glm\gtx\simd_glm::vec4.hpp>

SphereCollider::SphereCollider()
{
	m_colliderType = ColliderType::Sphere;
}

SphereCollider::SphereCollider(glm::vec4 position, float radius, ColliderTag tag, bool isTrigger, bool checkCollisionAgainstMesh)
{
	m_position = position;
	m_radius = radius;
	m_colliderTag = tag;
	m_colliderType = ColliderType::Sphere;
	m_isTrigger = isTrigger;
	m_checkCollisionAgainstMesh = checkCollisionAgainstMesh;
}

bool SphereCollider::CheckCollision(Collider& collider, CollisionEvent& collisionEvent)
{
	glm::vec4 m_PointOfCollision;

	switch (collider.GetColliderType())
	{
	case ColliderType::AABB:
		m_isColliding = CollisionDetection::Collide(this, (AABBCollider*)&collider, m_PointOfCollision);
		break;
	case ColliderType::Capsule:
		m_isColliding = CollisionDetection::Collide(this, (CapsuleCollider*)&collider, m_PointOfCollision);
		break;
	case ColliderType::Sphere:
		m_isColliding = CollisionDetection::Collide(this, (SphereCollider*)&collider, collisionEvent.m_PointOfCollision, collisionEvent.m_nCollisionVector, collisionEvent.m_CollisionDepth);
		break;
	default:
		m_isColliding = false;
		break;
	}

	return m_isColliding;
}

void SphereCollider::CheckCollisionAgainstMesh(MeshCollider& meshCollider)
{
	if (meshCollider.m_GameObject == this->m_GameObject)
		return;

	if (m_GameObject->m_Static)
		return;

	CollisionEvent collisionEvent(this, &meshCollider);

	if (!CheckCollision(meshCollider.GetBounds(), collisionEvent))
		return;

	// Transform sphere positions into local space of mesh to check against
	glm::vec4 pos = m_GameObject->GetWorldMatrix() * (m_position);
	pos = inverse(meshCollider.m_GameObject->m_Transform) * pos;
	SphereCollider sphere(pos, m_radius, ColliderTag::Other);
	sphere.m_GameObject = m_GameObject;

	// Collision Check - 0 if no collisions
	int collision = 0;

	glm::vec3 colPoint(FLT_MAX, FLT_MAX, FLT_MAX);
	float minDepth = FLT_MAX;
	bool collided = false;
	glm::vec4 dir;
	glm::vec3 normal;

	// Check spheres against all tris in the mesh
	for (unsigned int i = 0; i < meshCollider.triangles.size(); ++i)
	{
		glm::vec3 collisionPoint;
		Triangle triangle = meshCollider.triangles[i];
		collision = CollisionDetection::TestSphereTriangle(sphere, triangle, collisionPoint);

		if (collision > 0)
		{
			glm::vec3 worldCollisionPoint = meshCollider.m_GameObject->m_Transform * glm::vec4(collisionPoint, 1.0f);
			glm::vec4 direction = (m_GameObject->GetWorldMatrix() * (m_position)) - glm::vec4(worldCollisionPoint, 1.0f);
			float depth = m_radius - dot(direction, glm::vec4(triangle.m_Normal, 1.0f));

			if (depth < minDepth)
			{
				minDepth = depth;
				colPoint = collisionPoint;
				normal = triangle.m_Normal;
				dir = direction;
			}

			collided = true;
		}
	}

	if (collided)
	{
		collisionEvent.m_nCollisionVector = normalize(dir);
		collisionEvent.m_FaceNormal = normal;
		collisionEvent.m_CollisionDepth = minDepth;
		collisionEvent.m_PointOfCollision = glm::vec4(colPoint, 1);

		for (auto iter = m_onCollisionEnterEventList.begin(); iter != m_onCollisionEnterEventList.end(); ++iter)
		{
			(*iter)(collisionEvent);
		}
	}
}

const glm::vec4 SphereCollider::GetPosition()
{
	glm::vec4 position;

	//m_Mutex.lock();
	position = m_position;
	//m_Mutex.unlock();

	return position;
}

const glm::vec4 SphereCollider::GetWorldPosition()
{
	return m_GameObject->GetWorldMatrix() * m_position;
}

const float SphereCollider::GetRadius()
{
	float radius;

	//m_Mutex.lock();
	radius = m_radius;
	//m_Mutex.unlock();

	return radius;
}

void SphereCollider::SetPosition(glm::vec4 position)
{
	//m_Mutex.lock();
	m_position = position;
	//m_Mutex.unlock();
}

void SphereCollider::SetRadius(float radius)
{
	//m_Mutex.lock();
	m_radius = radius;
	//m_Mutex.unlock();
}

SphereCollider::~SphereCollider()
{
}