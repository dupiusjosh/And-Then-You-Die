#pragma once
#include <map>
#include <vector>
#include <string>
#include "Utils.h"
#include <fbxsdk.h>

#include "Mesh.h"
#include "AnimationData.h"
#include <mutex>
#define USE_DIRECTX_RENDERER
#define LOADERROR ~0U

#ifdef USE_DIRECTX_RENDERER
struct ID3D11Device;
struct D3D11_INPUT_ELEMENT_DESC;
struct ID3D11InputLayout;
#endif

#ifdef USE_VULKAN_RENDERER
#endif
struct Data
{
	enum class DataType
	{
		Invalid = -1,
		MeshData,
		AudioData,
		TextureData,
		numTypes
	};

	DataType m_Type;

	void* m_Data;

	Data() { m_Type = DataType::Invalid, m_Data = nullptr; }
	~Data();
};

struct ShaderData
{
	enum class ShaderType
	{
		Invalid = -1,
		VertexShader,
		HullShader,
		DomainShader,
		GeometryShader,
		PixelShader,
		ComputeShader,
		numTypes,
	};

	ShaderType m_Type;
	void* m_Shader;

	ShaderData() { m_Type = ShaderType::Invalid, m_Shader = nullptr; }
	~ShaderData();
};

class DataManager
{
	std::map<std::string, size_t> m_CachedId;
	std::map<std::string, size_t> m_CachedShaderId;
	std::mutex threadLock;
	std::vector<Data*> m_Data;
	std::vector<ShaderData*> m_Shaders;
	void LoadFbxAniNode(FbxNode* node, FbxTime time, Mesh * mesh, AnimationData::Frame * frame, AnimationData *ani);
	Mesh * LoadMeshFromNode(FbxNode * fnode);
	DataManager();
public:
	~DataManager();

	//Pretransform will remove an objects one structure and animations
	//Return value: Index of loaded mesh. Used by: Get(), LoadAnimation()
	size_t LoadMesh(char* path, bool batchable = false);
	//Return value: True if success
	size_t LoadAnimation(char* path, size_t meshId, char* name);
	//Return value: Index of loaded soundclip. Used by: Get()
	size_t LoadSound(char* path);
	//Return value: Index of loaded texture. Used by: Get()
	size_t LoadTexture(const char* path);
	//Return value: Index of loaded texture. Used by: Get()
	size_t LoadTexture(char * name, uint8_t * data, size_t len);

#ifdef USE_DIRECTX_RENDERER
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadVertexShader(const char* path, ID3D11Device*, ID3D11InputLayout*&, const D3D11_INPUT_ELEMENT_DESC*, uint16_t numElements);
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadHullShader(const char* path, ID3D11Device*);
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadDomainShader(const char* path, ID3D11Device*);
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadGeometryShader(const char* path, ID3D11Device*);
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadPixelShader(const char* path, ID3D11Device*);
	//Return value: Index of loaded shader. Used by: GetShader()
	size_t LoadComputeShader(const char* path, ID3D11Device*);
#endif

	char* GetAssetPath() { return "res/"; };
	std::string GetAssetPath(const char* dir);


	static DataManager* const GetInstance();

	void* Get(size_t);
	void* Get(size_t, Data::DataType);

	void* GetShader(size_t);
	void* GetShader(size_t, ShaderData::ShaderType);

	void ClearData();
	void ClearShaders();

};

