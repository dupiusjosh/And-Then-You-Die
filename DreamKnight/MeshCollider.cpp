#include "MeshCollider.h"

MeshCollider::MeshCollider()
{
	m_built = true;
	m_colliderType = ColliderType::Mesh;
}

MeshCollider::MeshCollider(Mesh * mesh)
{
	m_mesh = mesh;
	BuildTriangles();
	m_built = true;
	m_colliderType = ColliderType::Mesh;
}

void MeshCollider::OnStart()
{
	m_bounds.m_GameObject = m_GameObject;
}

void MeshCollider::OnUpdate(float deltaTime)
{
	return;
}

void MeshCollider::BuildTriangles()
{
	glm::vec4 min = glm::vec4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
	glm::vec4 max = glm::vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

	for (size_t i = 0; i < m_mesh->m_NumIndicies; i += 3)
	{
		// Get all 3 points to build each tri
		glm::vec3 p1 = m_mesh->m_VertexBufferInfo[m_mesh->m_IndexBufferInfo[i + 0]].Position;
		glm::vec3 p2 = m_mesh->m_VertexBufferInfo[m_mesh->m_IndexBufferInfo[i + 1]].Position;
		glm::vec3 p3 = m_mesh->m_VertexBufferInfo[m_mesh->m_IndexBufferInfo[i + 2]].Position;
		glm::vec4 normal = m_mesh->m_VertexBufferInfo[m_mesh->m_IndexBufferInfo[i + 0]].Normal;
		triangles.push_back(Triangle(p1, p2, p3, normal));

		for (unsigned int j = 0; j < 3; j++)
		{
			if (p1[j] > max[j])
				max[j] = p1[j];
			if (p2[j] > max[j])
				max[j] = p2[j];
			if (p3[j] > max[j])
				max[j] = p3[j];

			if (p1[j] < min[j])
				min[j] = p1[j];
			if (p2[j] < min[j])
				min[j] = p2[j];
			if (p3[j] < min[j])
				min[j] = p3[j];
		}
	}

	m_bounds.SetMax(max);
	m_bounds.SetMin(min);
}

const Mesh& MeshCollider::GetMesh()
{
	return *m_mesh;
}

AABBCollider & MeshCollider::GetBounds()
{
	return m_bounds;
}

MeshCollider::~MeshCollider()
{
}