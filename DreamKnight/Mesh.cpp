#include "Mesh.h"
#include "AnimationData.h"
#include "glm.h"
#include <d3d11.h>

Mesh::Mesh()
{
	//m_VertexBufferInfo = nullptr;
	//m_IndexBufferInfo = nullptr;
	//m_BoneOffsets = nullptr;
	m_NumIndicies = 0;
	m_NumVerticies = 0;
	m_Shader = -1;
}

Mesh::~Mesh()
{
	//delete[] m_VertexBufferInfo;
	//delete[] m_IndexBufferInfo;
	//delete[] m_BoneOffsets;
/*
	m_VertexBufferInfo = nullptr;
	m_IndexBufferInfo = nullptr;
	m_BoneOffsets = nullptr;*/


	//auto iter = m_Animations.begin();
	//while (iter != m_Animations.end())
	//{
	//	delete iter->second;
	//	iter->second = nullptr;
	//}

	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}
	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = nullptr;
	}

	for (auto p : m_Animations)
	{
		delete p.second;
	}
}

void Mesh::SetShader(int id)
{
	m_Shader = id;
}

void Mesh::FlipHands()
{
	glm::mat4x4 rotation(1);
	glm::rotate(rotation, 180.0f, glm::vec3(0, 1, 0));
	for (unsigned int i = 0; i < m_NumVerticies; ++i)
	{
		m_VertexBufferInfo[i].Position = rotation * m_VertexBufferInfo[i].Position;
	}
}
