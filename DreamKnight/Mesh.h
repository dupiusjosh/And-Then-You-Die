#pragma once
#include "glm/common.hpp"
#include "glm/mat4x4.hpp"
#include <unordered_map>

//TODO: Mesh: Replace Placeholder Types
struct TextureData;
struct AnimationData;

struct ID3D11Buffer;

enum RenderType
{
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	LINE_LIST,
	LINE_STRIP,
	BONES,
};

struct Mesh
{
	struct Vertex
	{
		glm::vec4 Position;
		glm::vec4 UV;
		glm::vec4 Normal;
		glm::vec4 Tangent;
		glm::vec4 Bitangent;
		glm::u32vec4 BoneIndicies;
		glm::vec4 BoneWeights;

		Vertex(glm::vec4 Position = glm::vec4(0, 0, 0, 1),
			glm::vec4 UV = glm::vec4(0, 0, 0, 0),
			glm::vec4 Normal = glm::vec4(0, 0, 0, 0),
			glm::vec4 Tangent = glm::vec4(0, 0, 0, 0),
			glm::vec4 Bitangent = glm::vec4(0, 0, 0, 0),
			glm::vec4 BoneIndicies = glm::u32vec4(0, 0, 0, 0),
			glm::vec4 BoneWeights = glm::vec4(0, 0, 0, 0)) : Position(Position), UV(UV), Normal(Normal), Tangent(Tangent), Bitangent(Bitangent), BoneIndicies(BoneIndicies), BoneWeights(BoneWeights)
		{}
	};

	std::vector<Vertex> m_VertexBufferInfo;
	std::vector<uint32_t> m_IndexBufferInfo;
	
	std::vector<glm::mat4x4> m_BoneOffsets;

	ID3D11Buffer *m_VertexBuffer = nullptr;
	ID3D11Buffer *m_IndexBuffer = nullptr;
	uint32_t m_NumVerticies;
	uint32_t m_NumIndicies;
	uint32_t m_NumBones;
	RenderType m_Type;

	int m_Texture = -1;

	int m_Shader;

	bool m_Batchable = false;

	std::unordered_map<std::string, AnimationData*> m_Animations;
	std::unordered_map<std::string, unsigned int> m_BoneNameToIndex;

	Mesh();
	~Mesh();

	void SetShader(int id);

	///Converts Right handed meshes to left handed, and vice-versa
	///Rotates All Positions in the Vertex Buffer 180 degrees on the Y axis
	void FlipHands();
};

