#include "MeshComponent.h"

MeshComponent::MeshComponent(Mesh * mesh)
{
	m_mesh = mesh;
}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::OnUpdate(float deltaTime)
{

}

Mesh* MeshComponent::GetRenderable(Renderer* renderer)
{
	if (m_mesh && m_mesh->m_IndexBuffer && m_mesh->m_VertexBuffer)
		return m_mesh;
	
	m_mesh = renderer->CreateRenderable(m_mesh);
	return m_mesh;
}

Mesh * MeshComponent::GetMesh()
{
	return m_mesh;
}

void MeshComponent::SetMesh(Mesh * newMesh)
{
	m_mesh = newMesh;
}
