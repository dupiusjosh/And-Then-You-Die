#include "AABBCollider.h"
#include "MeshCollider.h"
#include "CollisionDetection.h"
#include "Renderer.h"

AABBCollider::AABBCollider()
{
	//m_isStatic = false;
	m_checkCollisionAgainstMesh = false;
	m_colliderType = ColliderType::AABB;
}

AABBCollider::AABBCollider(glm::vec4 min, glm::vec4 max, ColliderTag tag, bool isTrigger, bool checkCollisionAgainstMesh)
{
	m_colliderType = ColliderType::AABB;
	m_min = min;
	m_max = max;
	m_colliderTag = tag;
	m_isTrigger = isTrigger;
	m_checkCollisionAgainstMesh = checkCollisionAgainstMesh;
}

bool AABBCollider::CheckCollision(Collider& collider, CollisionEvent& collisionEvent)
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
		m_isColliding = CollisionDetection::Collide(this, (SphereCollider*)&collider, m_PointOfCollision);
		break;
	default:
		m_isColliding = false;
		break;
	}

	return m_isColliding;
}

void AABBCollider::CheckCollisionAgainstMesh(MeshCollider& meshCollider)
{
}

const glm::vec4 AABBCollider::GetMin()
{
	glm::vec4 min;

	//m_Mutex.lock();
	min = m_min;
	//m_Mutex.unlock();

	return min;
}

const glm::vec4 AABBCollider::GetWorldMin()
{
	if (!m_GameObject)
		return GetMin();

	//m_Mutex.lock();
	glm::vec4 worldPosition(m_GameObject->m_Transform[3].x + m_min.x, m_GameObject->m_Transform[3].y + m_min.y, m_GameObject->m_Transform[3].z + m_min.z, 1.0f);
	//m_Mutex.unlock();

	return worldPosition;
}

const glm::vec4 AABBCollider::GetMax()
{
	glm::vec4 max;

	//m_Mutex.lock();
	max = m_max;
	//m_Mutex.unlock();

	return max;
}

const glm::vec4 AABBCollider::GetWorldMax()
{

	if (!m_GameObject)
		return GetMax();

	//m_Mutex.lock();
	glm::vec4 worldPosition(m_GameObject->m_Transform[3].x + m_max.x, m_GameObject->m_Transform[3].y + m_max.y, m_GameObject->m_Transform[3].z + m_max.z, 1.0f);
	//m_Mutex.unlock();

	return worldPosition;
}

glm::vec4 AABBCollider::GetCenter() const
{
	return (m_min + m_max) * 0.5f;
}

void AABBCollider::DrawLines(glm::vec4 color)
{
	Renderer* renderer = Renderer::GetInstance();

	glm::vec3 NTL, NTR, NBL, NBR, FTL, FTR, FBL, FBR;

	glm::vec3 min = GetWorldMin(); //NTL
	glm::vec3 max = GetWorldMax(); //FBR

	NTL = glm::vec3(min.x, min.y, min.z);
	NTR = glm::vec3(min.x, min.y, max.z);
	NBL = glm::vec3(min.x, max.y, min.z);
	NBR = glm::vec3(min.x, max.y, max.z);

	FTL = glm::vec3(max.x, min.y, min.z);
	FTR = glm::vec3(max.x, min.y, max.z);
	FBL = glm::vec3(max.x, max.y, min.z);
	FBR = glm::vec3(max.x, max.y, max.z);

	renderer->AddLine(NTL, NTR, color);
	renderer->AddLine(NTR, NBR, color);
	renderer->AddLine(NBR, NBL, color);
	renderer->AddLine(NBL, NTL, color);

	renderer->AddLine(NTL, FTL, color);
	renderer->AddLine(NTR, FTR, color);
	renderer->AddLine(NBR, FBR, color);
	renderer->AddLine(NBL, FBL, color);

	renderer->AddLine(FTL, FTR, color);
	renderer->AddLine(FTR, FBR, color);
	renderer->AddLine(FBR, FBL, color);
	renderer->AddLine(FBL, FTL, color);
}

void AABBCollider::SetMin(glm::vec4 min)
{
	//m_Mutex.lock();
	m_min = min;
	//m_Mutex.unlock();
}

void AABBCollider::SetMax(glm::vec4 max)
{
	//m_Mutex.lock();
	m_max = max;
	//m_Mutex.unlock();
}

AABBCollider::~AABBCollider()
{

}