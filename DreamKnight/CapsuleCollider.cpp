#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "CollisionDetection.h"
#include "Mesh.h"
#include "MeshComponent.h"

CapsuleCollider::CapsuleCollider()
{
	m_colliderType = ColliderType::Capsule;
}

CapsuleCollider::CapsuleCollider(glm::vec4 start, glm::vec4 end, float radius, ColliderTag tag, bool isTrigger, bool checkCollisionAgainstMesh)
{
	m_colliderType = ColliderType::Capsule;
	m_start = start;
	m_end = end;
	m_radius = radius;
	m_colliderTag = tag;
	m_isTrigger = isTrigger;
	m_checkCollisionAgainstMesh = checkCollisionAgainstMesh;
}

bool CapsuleCollider::CheckCollision(Collider& collider, CollisionEvent& collisionEvent)
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


void CapsuleCollider::CheckCollisionAgainstMesh(MeshCollider& meshCollider)
{
	if (meshCollider.m_GameObject == this->m_GameObject)
		return;

	if (m_GameObject->m_Static)
		return;

	CollisionEvent collisionEvent(this, &meshCollider);

	glm::vec4 m_position;

	if (m_start.y < m_end.y)
		m_position = m_start;
	else
		m_position = m_end;

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


void CapsuleCollider::CheckCollisionAgainstMesh2(MeshCollider& meshCollider)
{
	CollisionEvent collisionEvent;

	if (!CheckCollision(meshCollider.GetBounds(), collisionEvent))
		return;

	// Transform sphere positions into local space of mesh to check against
	glm::vec4 capStart = m_GameObject->m_Transform * (m_start);
	glm::vec4 capEnd = m_GameObject->m_Transform * (m_end);

	capStart = inverse(meshCollider.m_GameObject->m_Transform) * capStart;
	capEnd = inverse(meshCollider.m_GameObject->m_Transform) * capEnd;

	SphereCollider s1(capStart, m_radius, m_colliderTag);
	SphereCollider s2(capEnd, m_radius, m_colliderTag);

	// Collision Check - 0 if no collisions
	int collision = 0;

	if (meshCollider.m_GameObject == this->m_GameObject)
		return;

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
		collision = CollisionDetection::TestCapsuleTriangle(s1, s2, meshCollider.triangles[i], collisionPoint);

		if (collision > 0)
		{
			glm::vec3 worldCollisionPoint = meshCollider.m_GameObject->m_Transform * glm::vec4(collisionPoint, 1.0f);
			glm::vec4 direction = m_GameObject->m_Transform[3] - glm::vec4(worldCollisionPoint, 1.0f);
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
		for (auto iter = m_onCollisionEnterEventList.begin(); iter != m_onCollisionEnterEventList.end(); ++iter)
		{
			//(*iter)(CollisionEvent(this, meshCollider, normalize(dir), normalize(normal), minDepth));
		}
	}
}

const glm::vec4 CapsuleCollider::GetStart()
{
	glm::vec4 start;

	//m_Mutex.lock();
	start = m_start;
	//m_Mutex.unlock();

	return start;
}

const glm::vec4 CapsuleCollider::GetEnd()
{
	glm::vec4 end;

	//m_Mutex.lock();
	end = m_end;
	//m_Mutex.unlock();

	return end;
}

const glm::vec4 CapsuleCollider::GetWorldStart()
{
	glm::vec4 worldPosition;

	//m_Mutex.lock();
	worldPosition = m_GameObject->GetWorldMatrix() * m_start;
	//m_Mutex.unlock();

	return worldPosition;
}

const glm::vec4 CapsuleCollider::GetWorldEnd()
{
	glm::vec4 worldPosition;

	//m_Mutex.lock();
	worldPosition = m_GameObject->GetWorldMatrix() * m_end;
	//m_Mutex.unlock();

	return worldPosition;
}

const float CapsuleCollider::GetRadius()
{
	float radius;

	//m_Mutex.lock();
	radius = m_radius;
	//m_Mutex.unlock();

	return radius;
}

void CapsuleCollider::SetStart(glm::vec4 start)
{
	//m_Mutex.lock();
	m_start = start;
	//m_Mutex.unlock();
}

void CapsuleCollider::SetEnd(glm::vec4 end)
{
	//m_Mutex.lock();
	m_end = end;
	//m_Mutex.unlock();
}

void CapsuleCollider::BuildCapsuleAroundMesh()
{
	Mesh* mesh = m_GameObject->m_MeshComp->GetMesh();
	glm::vec3 minimum(FLT_MAX), maximum(-FLT_MAX);

	for (size_t i = 0; i < mesh->m_NumVerticies; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			float currentValue = mesh->m_VertexBufferInfo[mesh->m_IndexBufferInfo[i]].Position[j];

			if (currentValue < minimum[j])
				minimum[j] = currentValue;
			else if (currentValue > maximum[j])
				maximum[j] = currentValue;
		}
	}

	float widthX = maximum.x - minimum.x;
	float widthY = maximum.y - minimum.y;

	float radius;

	if (widthX > widthY)
		radius = widthX * 0.5f;
	else
		radius = widthY  * 0.5f;

	glm::vec4 start(minimum.x + radius, minimum.y - radius, maximum.z - radius, 1.0f);
	glm::vec4 end(minimum.x + radius, minimum.y + radius, minimum.z + radius, 1.0f);

	m_start = start;
	m_end = end;
	m_radius = radius;
}

void CapsuleCollider::SetRadius(float radius)
{
	//m_Mutex.lock();
	m_radius = radius;
	//m_Mutex.unlock();
}

CapsuleCollider::~CapsuleCollider()
{
	m_onCollisionEnterEventList.clear();
	m_onCollisionStayEventList.clear();
	m_onCollisionExitEventList.clear();
}