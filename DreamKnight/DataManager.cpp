#include "DataManager.h"
#include "Mesh.h"
#include "AnimationData.h"
#include <vorbis\vorbisfile.h>
#include <vector>
#include <map>
#include "glm.h"
#include "AudioManager.h"
#include "Renderer.h"
#include <string.h>

//Assimp Inclusion
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
/*
#ifdef WIN32
#pragma comment(lib, "assimp_x86.lib")
#else					 
#pragma comment(lib, "assimp_x64.lib")
#endif
*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define DEBUGMESHLOAD 0
#ifdef USE_DIRECTX_RENDERER
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#endif

#ifdef USE_VULKAN_RENDERER
#endif

namespace
{
	static DataManager* singleton = nullptr;
}

ShaderData::~ShaderData()
{
	/*
	#ifdef USE_DIRECTX_RENDERER
		switch (m_Type)
		{
		case (ShaderType::VertexShader):
		{
			((ID3D11VertexShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		case (ShaderType::HullShader):
		{
			((ID3D11HullShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		case (ShaderType::DomainShader):
		{
			((ID3D11DomainShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		case (ShaderType::GeometryShader):
		{
			((ID3D11GeometryShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		case (ShaderType::PixelShader):
		{
			((ID3D11PixelShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		case (ShaderType::ComputeShader):
		{
			((ID3D11ComputeShader*)m_Shader)->Release();
			m_Shader = nullptr;
			break;
		}
		}
	#endif
	*/
#ifdef USE_VULKAN_RENDERER
#endif
	m_Type = ShaderType::Invalid;
}

Data::~Data()
{
	switch (m_Type)
	{
		case(DataType::MeshData):
		{
			delete (Mesh*)m_Data;
			m_Data = nullptr;
		}
		case(DataType::AudioData):
		{
			delete (Sound*)m_Data;
			m_Data = nullptr;
		}
		case(DataType::TextureData):
		{

		}
	}
	m_Type = DataType::Invalid;
}

std::string DataManager::GetAssetPath(const char * dir)
{
	if (dir[1] == ':')
	{
		return std::string(dir);
	}
	char* path = GetAssetPath();
	std::string sdir = std::string(dir);
	if (sdir.find(path) == 0)
		return sdir;
	return std::string(path) + sdir;
}

DataManager* const DataManager::GetInstance()
{
	if (singleton == nullptr)
	{
		singleton = new DataManager();
	}
	return singleton;
}

DataManager::DataManager()
{

}

DataManager::~DataManager()
{
	ClearData();
	ClearShaders();
}

void DataManager::LoadFbxAniNode(FbxNode* node, FbxTime time, Mesh * mesh, AnimationData::Frame * frame, AnimationData *ani)
{

	//Load this frame, map the frames to the correct index
	FbxAMatrix gmat = node->EvaluateGlobalTransform(time);
	glm::mat4 matrix;
	for (USHORT i = 0; i < 4; ++i)
		for (USHORT j = 0; j < 4; ++j)
		{
			matrix[j][i] = (float)gmat.Get(j, i);
		}
	//get ref to the bone's pos in itr


	std::string name = node->GetName();
	size_t pos = name.find("mixamorig:");
	if (pos != std::string::npos)
	{
		//Strip the name
		name.erase(pos, 10);
	}
	auto res = mesh->m_BoneNameToIndex.find(name);
	//unsigned int indx;
	//Check if itr is valid.
	if (res != mesh->m_BoneNameToIndex.end())
	{
		//valid bone.
		unsigned int indx = mesh->m_BoneNameToIndex[name];
		frame->bonePos[indx] = matrix;
	}
	else
	{
		//mesh->m_BoneNameToIndex[node->GetName()]
		//add to animation as a "special bone" #HoldenLogic
		//ani->m_NumBones++;
		auto rest = ani->m_SpecialBones.find(name);
		if (rest != ani->m_SpecialBones.end())
		{
			unsigned int indx = ani->m_SpecialBones[name];
			frame->bonePos[indx] = (matrix);
		}
		else
		{
			ani->m_SpecialBones[name] = frame->bonePos.size();
			frame->bonePos.push_back(matrix);
		}
	}


	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		//recurr
		LoadFbxAniNode(node->GetChild(i), time, mesh, frame, ani);
	}


}

Mesh * DataManager::LoadMeshFromNode(FbxNode * fnode)
{
	//FBX verts consist of 3 things
	//UVs, Normals, and control points
	FbxMesh * fmesh = fnode->GetMesh();
	if (fmesh == nullptr)
	{
		Mesh * mesh = nullptr;
		for (int i = 0; i < fnode->GetChildCount(); ++i)
		{
			mesh = LoadMeshFromNode(fnode->GetChild(i));
			if (mesh)
				break;
		}
		return mesh;
	}
	Mesh * m = new Mesh();
	//get the count of control points
	m->m_NumVerticies = fmesh->GetControlPointsCount();
	//FbxVector4 * verts = new FbxVector4[m->vertCount];
	std::vector<Mesh::Vertex> verts;
	for (unsigned int i = 0; i < m->m_NumVerticies; ++i)
	{

		Mesh::Vertex v;
		ZeroMemory(&v, sizeof(v));
		FbxVector4 vert = fmesh->GetControlPointAt(i);
		v.Position.x = (float)vert[0];
		v.Position.y = (float)vert[1];
		v.Position.z = (float)vert[2];
		v.Position.w = (float)vert[3];
		verts.push_back(v);
	}
	int indCnt = fmesh->GetPolygonVertexCount();
	int polyCnt = fmesh->GetPolygonCount();

	if (fnode->GetMaterialCount() > 0)
	{
		printf("%i\n", fnode->GetMaterialCount());
		auto mat = fnode->GetMaterial(0);

		//Diffuse - is the color texture
		//Ambient - Metal texture
		//Bumb - Roughness texture
		{
			const char *textureT = mat->sDiffuse;
			auto prop = mat->FindProperty(textureT);
			if (prop.IsValid())
			{
				int textureCnt = prop.GetSrcObjectCount<FbxTexture>();
				printf("DIFFUSE TEXTURE COUNT: %i\n", textureCnt);
				FbxTexture* texture = prop.GetSrcObject<FbxTexture>(0);
				if (texture)
				{

					FbxString type = texture->GetTextureType();
					//const char* type = type.Buffer();

					FbxFileTexture* file = FbxCast<FbxFileTexture>(texture);
					if (file)
					{
						std::string textureName = file->GetFileName();

						m->m_Texture = LoadTexture(textureName.c_str());
					}
					else
					{
						m->m_Texture = 0;
					}
				}
			}
		}
		{
			const char * textureT = mat->sAmbient;
			auto prop = mat->FindProperty(textureT);
			if (prop.IsValid())
			{

				int textureCnt = prop.GetSrcObjectCount<FbxTexture>();
				printf("AMBIENT TEXTURE COUNT: %i\n", textureCnt);
				//FbxTexture* texture = prop.GetSrcObject<FbxTexture>(0);
				//if (texture)
				//{
				//	FbxString type = texture->GetTextureType();
				//	//const char* type = type.Buffer();

				//	FbxFileTexture* file = FbxCast<FbxFileTexture>(texture);
				//	if (file)
				//	{
				//		string textureName = file->GetFileName();
				//		printf("LOADING TEXTURE: %s", textureName.c_str());

				//		m->m_Texture = LoadTexture(textureName.c_str());
				//	}
				//}
			}

		}

		{
			const char * textureT = mat->sBump;
			auto prop = mat->FindProperty(textureT);
			if (prop.IsValid())
			{

				int textureCnt = prop.GetSrcObjectCount<FbxTexture>();
				printf("Bump TEXTURE COUNT: %i\n", textureCnt);
				//FbxTexture* texture = prop.GetSrcObject<FbxTexture>(0);
				//if (texture)
				//{

				//	FbxString type = texture->GetTextureType();
				//	//const char* type = type.Buffer();

				//	FbxFileTexture* file = FbxCast<FbxFileTexture>(texture);
				//	if (file)
				//	{
				//		string textureName = file->GetFileName();

				//		m->m_Texture = LoadTexture(textureName.c_str());
				//	}
				//}
			}

		}
	}


	unsigned int deformerCnt = fmesh->GetDeformerCount();
	fbxsdk::FbxSkin * skin = (fbxsdk::FbxSkin*)fmesh->GetDeformer(0, FbxDeformer::eSkin);
	int * boneVertCount = new int[m->m_NumVerticies];
	ZeroMemory(boneVertCount, sizeof(int) * m->m_NumVerticies);
	//check to make sure we have skin
	if (skin)
	{

		//vertBones = new DirectX::XMFLOAT4[m->vertCount];
		//vertWeights = new DirectX::XMFLOAT4[m->vertCount];
		//vertex groups


		int boneCnt = skin->GetClusterCount();
		m->m_NumBones = boneCnt;
		//Loop vertex groups
		for (int bIndex = 0; bIndex < boneCnt; ++bIndex)
		{
			//individual bone group
			fbxsdk::FbxCluster * cluster = skin->GetCluster(bIndex);
			//Bone this cluster is for.
			FbxNode* bone = cluster->GetLink();
			printf("%s \n", bone->GetName());

			//Get the bind pos mat for this bone cluster
			FbxAMatrix bindPos;
			glm::mat4 matrix;
			cluster->GetTransformLinkMatrix(bindPos);
			for (USHORT i = 0; i < 4; ++i)
				for (USHORT j = 0; j < 4; ++j)
				{

					matrix[j][i] = (float)bindPos.Get(j, i);

				}
			matrix = glm::inverse(matrix);
			//get collections of the stuff.
			int* boneVertIndx = cluster->GetControlPointIndices();
			double* boneVertWeight = cluster->GetControlPointWeights();

			//This needs to be what verts are effected by what bones
			//Let bones know who they affect
			unsigned int clusterVertCnt = cluster->GetControlPointIndicesCount();
			for (unsigned int vIndex = 0; vIndex < clusterVertCnt; ++vIndex)
			{
				//thisB.vertIDs.push_back(boneVertIndx[vIndex]);
				//thisB.boneWeight.push_back((float)boneVertWeight[vIndex]);

				//what bone is this for the vert, assign it the next bone to be added.
				/*if (temporaryVerts[boneVertIndx[vIndex]].bneCnt < 4)
				{
				temporaryVerts[boneVertIndx[vIndex]].boneID[temporaryVerts[boneVertIndx[vIndex]].bneCnt] = m.bones.size();
				temporaryVerts[boneVertIndx[vIndex]].boneWeight[temporaryVerts[boneVertIndx[vIndex]].bneCnt] = boneVertWeight[vIndex];
				temporaryVerts[boneVertIndx[vIndex]].bneCnt++;
				}*/
				if (boneVertCount[boneVertIndx[vIndex]] < 4)
				{
					verts[boneVertIndx[vIndex]].BoneIndicies[boneVertCount[boneVertIndx[vIndex]]] = m->m_BoneOffsets.size();// boneVertCount[boneVertIndx[vIndex]];
					verts[boneVertIndx[vIndex]].BoneWeights[boneVertCount[boneVertIndx[vIndex]]] = (float)boneVertWeight[vIndex];
					boneVertCount[boneVertIndx[vIndex]]++;
				}
				else
				{
					printf("BONE CNT OVER 4 %f\n", boneVertWeight[vIndex]);
					int indxToChange = -1;
					for (unsigned int vbind = 0; vbind < 4; vbind++)
					{
						if (verts[boneVertIndx[vIndex]].BoneWeights[vbind] < (float)boneVertWeight[vIndex])
						{
							if (indxToChange != -1)
							{
								if (verts[boneVertIndx[vIndex]].BoneWeights[vbind] < verts[boneVertIndx[vIndex]].BoneWeights[indxToChange])
								{
									indxToChange = vbind;
								}
							}
							else
							{
								indxToChange = vbind;
							}
						}
					}
					if (indxToChange > 0)
					{
						verts[boneVertIndx[vIndex]].BoneIndicies[indxToChange] = m->m_BoneOffsets.size();// boneVertCount[boneVertIndx[vIndex]];
						printf("replacing weight: %f with: %f\n", verts[boneVertIndx[vIndex]].BoneWeights[indxToChange], (float)boneVertWeight[vIndex]);
						verts[boneVertIndx[vIndex]].BoneWeights[indxToChange] = (float)boneVertWeight[vIndex];
						//break;
					}
				}
			}

			std::string name = bone->GetName();
			size_t pos = name.find("mixamorig:");
			if (pos != std::string::npos)
			{
				//Strip the name
				name.erase(pos, 10);
			}
			m->m_BoneNameToIndex[name.c_str()] = m->m_BoneOffsets.size();
			//m->m_BoneNameToIndex[bone->GetName()] = m->m_BoneOffsets.size();
			//m.bones.push_back(thisB);
			m->m_BoneOffsets.push_back(matrix);
		}

	}

	FbxStringList uvsetlist;
	fmesh->GetUVSetNames(uvsetlist);
	uint64_t currVIndex = 0;
	for (int p = 0; p < polyCnt; ++p)
	{
		for (int i = 0; i < 3; i++)
		{
			FbxVector2 uv;
			bool unmapped;
			FbxVector4 normal;
			fmesh->GetPolygonVertexUV(p, i, uvsetlist.GetStringAt(0), uv, unmapped);
			int indx = fmesh->GetPolygonVertex(p, i);
			fmesh->GetPolygonVertexNormal(p, i, normal);

			Mesh::Vertex vert;
			vert.Position.x = verts[indx].Position.x;
			vert.Position.y = verts[indx].Position.y;
			vert.Position.z = verts[indx].Position.z;
			vert.Position.w = 1;
			vert.UV.x = (float)uv[0];
			vert.UV.y = (float)uv[1];
			vert.UV.z = 0;
			vert.UV.w = 1;
			vert.Normal.x = (float)normal.mData[0];
			vert.Normal.y = (float)normal.mData[1];
			vert.Normal.z = (float)normal.mData[2];
			vert.Normal.w = 0;
			//vert.boneid
			//vert.bonewgt
			//vert.BoneCnt = verts[indx].BoneCnt;
			for (unsigned int j = 0; j < 4; ++j)
			{
				vert.BoneIndicies[j] = verts[indx].BoneIndicies[j];
				vert.BoneWeights[j] = verts[indx].BoneWeights[j];
			}
			m->m_IndexBufferInfo.push_back(m->m_VertexBufferInfo.size());
			m->m_VertexBufferInfo.push_back(vert);
		}
	}
	delete[] boneVertCount;

	m->m_NumIndicies = m->m_IndexBufferInfo.size();
	return m;
}

size_t DataManager::LoadMesh(char* dir, bool batchable)
{
	threadLock.lock();
#ifndef NDEBUG
	//printf_s("Loading Mesh: \n");
	//printf_s("Path - %s\n", path);
#endif

	if (m_CachedId.find(dir) != m_CachedId.end())
	{
		threadLock.unlock();

		return (m_CachedId[dir] - 1);
	}

	if (dir != nullptr && dir[0] == '#')
	{
		Data* newData = new Data();
		newData->m_Type = Data::DataType::MeshData;
		newData->m_Data = new Mesh();
		m_Data.push_back(newData);

		m_CachedId[dir] = m_Data.size();
		threadLock.unlock();
		return m_Data.size() - 1;

	}

	std::string path = GetAssetPath(dir);

	//Create manager
	FbxManager * manager = FbxManager::Create();
	//Settings setup
	FbxIOSettings * ios = FbxIOSettings::Create(manager, IOSROOT);
	//Set the settings
	manager->SetIOSettings(ios);
	//setup the importer used to load the fbx in scene
	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(path.c_str(), -1, manager->GetIOSettings()))
	{
		//printf("Call to FbxImporter::Initialize() failed.\n");
		threadLock.unlock();
		//printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return -1;
	}
	//Setup the scene
	FbxScene * scene = FbxScene::Create(manager, "sceneName");
	//import into the scene
	importer->Import(scene);
	//destroy the importer, we no longer need it now that data is in the scene
	importer->Destroy();
	//FbxSystemUnit::m.ConvertScene(scene);

	//Geo manager in order to force tri's on the mesh
	FbxGeometryConverter conv(manager);
	if (conv.Triangulate(scene, true) == false)
	{
		threadLock.unlock();
		//if failed, return;
		return -1;
	}

	//Alright! Lets get the root node now.
	FbxNode * rNode = scene->GetRootNode();
	//init ret value
	int ret = LOADERROR;
	//Recur function to go through nodes for first mesh.
	Mesh * m = LoadMeshFromNode(rNode);
	if (m)
	{
		m->m_NumVerticies = m->m_VertexBufferInfo.size();
		//The loaded mesh
		Data* newData = new Data();
		newData->m_Type = Data::DataType::MeshData;
		newData->m_Data = m;
		m->m_Batchable = batchable;
		m_Data.push_back(newData);
		m_CachedId[dir] = m_Data.size();
		ret = m_Data.size() - 1;
	}

	if (ret == LOADERROR)
	{
		assert("Scene has no Meshes");
	}
	manager->Destroy();

	if (ret == LOADERROR)
		assert("Scene Load Error");
	threadLock.unlock();

	return (ret);
	//return LOADERROR;
}

//DEPRICATION
//size_t DataManager::LoadMesh_OLD(char* dir, bool pretransformedVerticies)
//{
//
//#ifndef NDEBUG
//	//printf_s("Loading Mesh: \n");
//	//printf_s("Path - %s\n", path);
//#endif
//
//	if (m_CachedId.find(dir) != m_CachedId.end())
//	{
//		return (m_CachedId[dir] - 1);
//	}
//
//	if (dir != nullptr && dir[0] == '#')
//	{
//		Data* newData = new Data();
//		newData->m_Type = Data::DataType::MeshData;
//		newData->m_Data = new Mesh();
//		m_Data.push_back(newData);
//
//		m_CachedId[dir] = m_Data.size();
//		return m_Data.size() - 1;
//	}
//
//	string path = GetAssetPath(dir);
//
//	Assimp::Importer importer;
//	const aiScene* scene;
//	uint32_t flags;
//	flags = aiPostProcessSteps::aiProcess_JoinIdenticalVertices |
//		aiPostProcessSteps::aiProcess_MakeLeftHanded |
//		aiPostProcessSteps::aiProcess_FlipWindingOrder |
//		aiPostProcessSteps::aiProcess_CalcTangentSpace |
//		aiPostProcessSteps::aiProcess_Triangulate |
//		aiPostProcessSteps::aiProcess_SortByPType;
//	if (pretransformedVerticies)
//	{
//		flags |= aiPostProcessSteps::aiProcess_PreTransformVertices;
//	}
//
//	scene = importer.ReadFile(path, flags);
//
//	std::string errorString = importer.GetErrorString();
//
//	if (scene != nullptr)
//	{
//		size_t ret = LOADERROR;
//		if (scene->HasMeshes())
//		{
//
//			Mesh* newMesh = new Mesh();
//			aiMesh* LoadingMesh = scene->mMeshes[0];
//
//			newMesh->m_NumVerticies = LoadingMesh->mNumVertices;
//			newMesh->m_VertexBufferInfo = new Mesh::Vertex[LoadingMesh->mNumVertices];
//
//			if (LoadingMesh->HasPositions())
//			{
//				for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
//				{
//					newMesh->m_VertexBufferInfo[i].Position.x = LoadingMesh->mVertices[i].x;
//					newMesh->m_VertexBufferInfo[i].Position.y = LoadingMesh->mVertices[i].y;
//					newMesh->m_VertexBufferInfo[i].Position.z = LoadingMesh->mVertices[i].z;
//					newMesh->m_VertexBufferInfo[i].Position.w = 1;
//				}
//			}
//
//			if (LoadingMesh->HasNormals())
//			{
//				for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
//				{
//					newMesh->m_VertexBufferInfo[i].Normal.x = LoadingMesh->mNormals[i].x;
//					newMesh->m_VertexBufferInfo[i].Normal.y = LoadingMesh->mNormals[i].y;
//					newMesh->m_VertexBufferInfo[i].Normal.z = LoadingMesh->mNormals[i].z;
//					newMesh->m_VertexBufferInfo[i].Normal.w = 0;
//				}
//			}
//
//			//Has texture coords on channel 0
//			if (LoadingMesh->HasTextureCoords(0))
//			{
//				for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
//				{
//
//					//Channel 0 texture coords
//					newMesh->m_VertexBufferInfo[i].UV.x = LoadingMesh->mTextureCoords[0][i].x;
//					newMesh->m_VertexBufferInfo[i].UV.y = LoadingMesh->mTextureCoords[0][i].y;
//					newMesh->m_VertexBufferInfo[i].UV.z = LoadingMesh->mTextureCoords[0][i].z;
//					newMesh->m_VertexBufferInfo[i].UV.w = 1;
//				}
//			}
//
//			if (scene->HasTextures())
//			{
//				uint32_t idx = LoadingMesh->mMaterialIndex;
//				if (scene->mMaterials[idx]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
//				{
//#ifdef WIN32
//					aiString tex_path;
//					scene->mMaterials[idx]->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path);
//					if (scene->mTextures[0]->mHeight == 0)
//					{
//						size_t buff_len = path.length() + 32;
//						char* buff = (char*)malloc(buff_len);
//						sprintf_s(buff, buff_len, "%s: %s", path.c_str(), tex_path.C_Str());
//						newMesh->m_Texture = LoadTexture(buff, (uint8_t*)scene->mTextures[0]->pcData, scene->mTextures[0]->mWidth);
//						free(buff);
//					}
//#endif
//				}
//			}
//
//			if (LoadingMesh->HasTangentsAndBitangents())
//			{
//				for (unsigned int i = 0; i < LoadingMesh->mNumVertices; ++i)
//				{
//					newMesh->m_VertexBufferInfo[i].Tangent.x = LoadingMesh->mTangents[i].x;
//					newMesh->m_VertexBufferInfo[i].Tangent.y = LoadingMesh->mTangents[i].y;
//					newMesh->m_VertexBufferInfo[i].Tangent.z = LoadingMesh->mTangents[i].z;
//					newMesh->m_VertexBufferInfo[i].Tangent.w = 0;
//
//					newMesh->m_VertexBufferInfo[i].Bitangent.x = LoadingMesh->mBitangents[i].x;
//					newMesh->m_VertexBufferInfo[i].Bitangent.y = LoadingMesh->mBitangents[i].y;
//					newMesh->m_VertexBufferInfo[i].Bitangent.z = LoadingMesh->mBitangents[i].z;
//					newMesh->m_VertexBufferInfo[i].Bitangent.w = 0;
//				}
//			}
//
//			if (LoadingMesh->HasFaces())
//			{
//				newMesh->m_NumIndicies = LoadingMesh->mNumFaces * 3;
//				newMesh->m_IndexBufferInfo = new unsigned int[newMesh->m_NumIndicies];
//
//				for (unsigned int i = 0, limit = LoadingMesh->mNumFaces; i < limit; ++i)
//				{
//					for (unsigned int j = 0; j < 3U; ++j)
//					{
//						newMesh->m_IndexBufferInfo[(i * 3) + j] = LoadingMesh->mFaces[i].mIndices[j];
//					}
//				}
//			}
//
//			if (LoadingMesh->HasBones())
//			{
//				newMesh->m_NumBones = LoadingMesh->mNumBones;
//				newMesh->m_BoneOffsets = new glm::mat4x4[LoadingMesh->mNumBones];
//
//				unsigned int* numBones = new unsigned int[newMesh->m_NumVerticies]{ 0 };
//				for (unsigned int i = 0; i < LoadingMesh->mNumBones; ++i)
//				{
//					newMesh->m_BoneNameToIndex[std::string(LoadingMesh->mBones[i]->mName.data)] = i;
//					/*for (unsigned int j = 0; j < 4; ++j)
//					{
//						for (unsigned int k = 0; k < 4; ++k)
//						{
//							newMesh->m_BoneOffsets[i][j][k] = LoadingMesh->mBones[i]->mOffsetMatrix[k][j];
//						}
//					}*/
//					{
//						auto thing = LoadingMesh->mBones[i]->mOffsetMatrix;
//						newMesh->m_BoneOffsets[i] = (glm::mat4(
//							thing.a1, thing.b1, thing.c1, thing.d1,
//							thing.a2, thing.b2, thing.c2, thing.d2,
//							thing.a3, thing.b3, thing.c3, thing.d3,
//							thing.a4, thing.b4, thing.c4, thing.d4));
//					}
//					for (int j = 0, limit = LoadingMesh->mBones[i]->mNumWeights;
//						j < limit; 
//						++j)
//					{
//						int vertID = LoadingMesh->mBones[i]->mWeights[j].mVertexId;
//						if (numBones[vertID] < 4)
//						{
//							newMesh->m_VertexBufferInfo[vertID].BoneIndicies[numBones[vertID]] = i;
//							newMesh->m_VertexBufferInfo[vertID].BoneWeights[numBones[vertID]] = LoadingMesh->mBones[i]->mWeights[j].mWeight;
//							++numBones[vertID];
//						}
//						
//					}
//				}
//
//				/*		auto inverseRoot = scene->mRootNode->mTransformation.Inverse();
//						glm::mat4 glmInverseRoot = glm::mat4(inverseRoot.a1, inverseRoot.a2, inverseRoot.a3, inverseRoot.a4,
//															 inverseRoot.b1, inverseRoot.b2, inverseRoot.b3, inverseRoot.b4,
//															 inverseRoot.c1, inverseRoot.c2, inverseRoot.c3, inverseRoot.c4,
//															 inverseRoot.d1, inverseRoot.d2, inverseRoot.d3, inverseRoot.d4);*/
//				//for (unsigned int i = 0; i < LoadingMesh->mNumBones; ++i)
//				//{
//				//	//if (newMesh->m_boneNameToIndex.find("Root_J")->second != i)
//				//	{
//				//		newMesh->m_BoneOffsets[i] *= glmInverseRoot;// *= newMesh->m_BoneOffsets[newMesh->m_boneNameToIndex["Root_J"]];
//				//	}
//				//}
//
//				//for (unsigned int i = 0; i < LoadingMesh->mNumBones; ++i)
//				//{
//				//	auto node = scene->mRootNode->FindNode(LoadingMesh->mBones[i]->mName);
//
//				//	if (!node) continue;
//				//	glm::mat4& boneMatrix = newMesh->m_BoneOffsets[newMesh->m_boneNameToIndex[LoadingMesh->mBones[i]->mName.data]];
//
//				//	while (node)
//				//	{
//				//		auto thing = node->mTransformation;
//
//				//		auto transform = (glm::mat4(
//				//			thing.a1, thing.b1, thing.c1, thing.d1,
//				//			thing.a2, thing.b2, thing.c2, thing.d2,
//				//			thing.a3, thing.b3, thing.c3, thing.d3,
//				//			thing.a4, thing.b4, thing.c4, thing.d4 ));
//				//		/*
//				//			thing.b1, thing.b2, thing.b3, thing.b4,
//				//			thing.c1, thing.c2, thing.c3, thing.c4,
//				//			thing.d1, thing.d2, thing.d3, thing.d4));
//				//			*/
//				//		(boneMatrix) *= transform;
//				//		node = node->mParent;
//				//	}
//				//}
//
//				//for (unsigned int i = 0; i < LoadingMesh->mNumBones; ++i)
//				//{
//				//	auto node = scene->mRootNode->FindNode(LoadingMesh->mBones[i]->mName);
//				//
//				//	std::vector<aiNode*> stuff;
//				//	while (node != scene->mRootNode)
//				//	{
//				//		stuff.push_back(node);
//				//		node = node->mParent;
//				//	}
//				//
//				//	aiMatrix4x4 thing = aiMatrix4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
//				//	
//				//	for (int j = 0; j < stuff.size(); ++j)
//				//	{
//				//		node = stuff[j];
//				//		//stuff.pop_back();
//				//
//				//		thing *= node->mTransformation;
//				//	}
//				//
//				//	auto glmThing = glm::mat4(thing.a1, thing.a2, thing.a3, thing.a4,
//				//							  thing.b1, thing.b2, thing.b3, thing.b4,
//				//							  thing.c1, thing.c2, thing.c3, thing.c4,
//				//							  thing.d1, thing.d2, thing.d3, thing.d4);
//				//	newMesh->m_BoneOffsets[i] = /*glmInverseRoot **/ glmThing * newMesh->m_BoneOffsets[i];// *= glmInverseRoot;
//				//	newMesh->m_BoneOffsets[i] = glm::inverse(newMesh->m_BoneOffsets[i]);
//				//}
//
//				delete[] numBones;
//
//			}
//
//
//
//#if DEBUGMESHLOAD == 1
//			for (unsigned int i = 0; i < newMesh->m_NumVerticies; ++i)
//			{
//				printf_s("Printing Vertex %i bone information:\n", i);
//				for (unsigned int j = 0; j < 4; ++j)
//				{
//					printf_s("\tIndex %i: %i\tWeight %i: %f", j, newMesh->m_VertexBufferInfo[i].BoneIndicies[j], j, newMesh->m_VertexBufferInfo[i].BoneWeights[j]);
//				}
//			}
//#endif
//#if DEBUGMESHLOAD == 2
//			for (unsigned int i = 0; i < newMesh->m_NumVerticies; ++i)
//			{
//				printf_s("Printing Vertex %i position information:\n", i);
//				printf_s("\tX: %f\tY: %f\tZ: %f\n", (float)newMesh->m_VertexBufferInfo[i].Position.x, (float)newMesh->m_VertexBufferInfo[i].Position.y, (float)newMesh->m_VertexBufferInfo[i].Position.z);
//			}
//#endif
//			Data* newData = new Data();
//			newData->m_Type = Data::DataType::MeshData;
//			newData->m_Data = newMesh;
//			m_Data.push_back(newData);
//
//			m_CachedId[dir] = m_Data.size();
//			ret = m_Data.size() - 1;
//		}
//
//		if (ret == LOADERROR)
//		{
//			assert("Scene has no Meshes");
//		}
//
//		importer.FreeScene();
//
//		return (ret);
//	}
//	assert("Scene Load Error");
//	return LOADERROR;
//}

size_t DataManager::LoadAnimation(char * dir, size_t meshId, char* name)
{
	threadLock.lock();

	std::string path = GetAssetPath(dir);

	Mesh * mesh = (Mesh*)Get(meshId, Data::DataType::MeshData);
	//Check if name is taken, or ani is already loaded.
	if (mesh->m_Animations.find(name) != mesh->m_Animations.end())
	{
		threadLock.unlock();
		//return LOADERROR;
		return 0;
	}
	//FBX stuff here.
	//Create manager
	FbxManager * manager = FbxManager::Create();
	//Settings setup
	FbxIOSettings * ios = FbxIOSettings::Create(manager, IOSROOT);
	//Set the settings
	manager->SetIOSettings(ios);
	//setup the importer used to load the fbx in scene
	FbxImporter* importer = FbxImporter::Create(manager, "");
	if (!importer->Initialize(path.c_str(), -1, manager->GetIOSettings()))
	{
		threadLock.unlock();
		//printf("Call to FbxImporter::Initialize() failed.\n");
		//printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return LOADERROR;
	}
	//Setup the scene
	FbxScene * scene = FbxScene::Create(manager, "sceneName");
	//import into the scene
	importer->Import(scene);
	//destroy the importer, we no longer need it now that data is in the scene
	importer->Destroy();

	//FbxSystemUnit::m.ConvertScene(scene);
	FbxCriteria crit = FbxCriteria::ObjectTypeStrict(FbxAnimStack::ClassId);
	int numAnimations = scene->GetSrcObjectCount(crit);

	if (!numAnimations)
	{
		threadLock.unlock();

		return LOADERROR;
	}

	//Only loading the first anim
	FbxAnimStack *animStack = (FbxAnimStack*)scene->GetSrcObject(crit, 0);
	FbxAnimEvaluator *animEvaluator = scene->GetAnimationEvaluator();
	//animEvaluator.set
	//Span of the animation
	FbxTimeSpan span = animStack->GetLocalTimeSpan();
	//Actual duration of the animation
	FbxTime durr = span.GetDuration();
	//FbxTime total = span.GetDuration();

	//time stamp
	FbxLongLong fll = durr.GetFrameCount(FbxTime::EMode::eFrames30);
	//Loop for frame count.
	AnimationData * ani = new AnimationData();
	//ani->m_DurationInSeconds = (durr.GetMilliSeconds()/1000.0f);
	FbxNode* node = scene->GetRootNode();
	node->ResetPivotSetAndConvertAnimation();
	ani->m_NumBones = mesh->m_NumBones;
	for (FbxLongLong i = 0; i < fll; ++i)
	{
		//i is frame number.
		durr.SetFrame(i, FbxTime::EMode::eFrames30);
		node = scene->GetRootNode();

		AnimationData::Frame frame;
		frame.timestampInSeconds = durr.GetMilliSeconds() / 1000.0f;
		//size the frame to the bone cnt of the mesh. Preempt.
		frame.bonePos.resize(ani->m_NumBones);
		//call load frame, recursive function
		LoadFbxAniNode(node, durr, mesh, &frame, ani);
		ani->m_NumBones = frame.bonePos.size();
		//load frame into the animation
		ani->frames.push_back(frame);
		ani->m_DurationInSeconds = frame.timestampInSeconds;
	}
	//animStack->GetName(); // Get the name of the animation if needed

	//Alright! Lets get the root node now.
	mesh->m_Animations[name] = ani;
	//Cleanup!
	manager->Destroy();
	threadLock.unlock();

	return 0;
}

//
////Path to fbx file of animation, meshid is for the bone index mapping
//size_t DataManager::LoadAnimation_Old(char * dir, size_t meshId)
//{
//	string path = GetAssetPath(dir);
//
//	Assimp::Importer importer;
//	const aiScene* scene;
//	uint32_t flags;
//	flags = aiPostProcessSteps::aiProcess_JoinIdenticalVertices |
//		aiPostProcessSteps::aiProcess_MakeLeftHanded |
//		aiPostProcessSteps::aiProcess_Triangulate;
//
//	scene = importer.ReadFile(path, flags);
//
//	std::string errorString = importer.GetErrorString();
//
//	if (scene != nullptr)
//	{
//		auto a = scene->mAnimations;
//		Mesh* m = (Mesh*)Get(meshId, Data::DataType::MeshData);
//
//		for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
//		{
//			AnimationData * aData = new AnimationData();
//
//			auto channels = a[i]->mChannels;
//			aData->m_NumBones = a[i]->mNumChannels;
//			aData->m_DurationInTicks = a[i]->mDuration;
//			if (a[i]->mTicksPerSecond != 0)
//			{
//				aData->m_TicksPerSecond = a[i]->mTicksPerSecond;
//			}
//			else
//			{
//				aData->m_TicksPerSecond = 30;
//			}
//			aData->m_DurationInSeconds = (aData->m_DurationInTicks / aData->m_TicksPerSecond);
//			aData->m_DurationPerTick = (aData->m_DurationInSeconds / aData->m_DurationInTicks);
//
//			std::map<double, glm::mat4> times;
//
//			for (unsigned int j = 0, limit = a[i]->mNumChannels; j < limit; ++j)
//			{
//				auto node = channels[j];
//
//				unsigned int key = 0;
//				for (; key < node->mNumPositionKeys; ++key)
//				{
//					times[node->mPositionKeys[key].mTime] = glm::mat4(1);
//				}
//				for (key = 0; key < node->mNumRotationKeys; ++key)
//				{
//					times[node->mRotationKeys[key].mTime] = glm::mat4(1);
//				}
//				for (key = 0; key < node->mNumScalingKeys; ++key)
//				{
//					times[node->mScalingKeys[key].mTime] = glm::mat4(1);
//				}
//			}
//
//			aData->frames.resize(times.size());
//			for (AnimationData::Frame& f : aData->frames)
//			{
//				f.bonePos.resize(a[i]->mNumChannels);
//			}
//
//			unsigned int offset = 0;
//			for (const auto& p : m->m_BoneNameToIndex)
//			{
//				if (p.second >= offset)
//				{
//					offset = p.second;
//				}
//			}
//
//			for (unsigned int j = 0; j < a[i]->mNumChannels; ++j)
//			{
//				auto node = a[i]->mChannels[j];
//				unsigned int posKey = 0, rotKey = 0, scaleKey = 0;
//
//				std::string name(node->mNodeName.C_Str());
//
//				unsigned int k;
//
//				if (m->m_BoneNameToIndex.find(name) == m->m_BoneNameToIndex.end())
//				{
//					aData->m_SpecialBones[name] = ++offset;
//					m->m_BoneNameToIndex[name] = offset;
//					k = offset;
//				}
//				else
//				{
//					k = m->m_BoneNameToIndex[name];
//				}
//
//				unsigned int index = 0;
//
//				glm::mat4 GlobalMatrix(1);
//
//				for (auto tempNode = scene->mRootNode->FindNode(name.data())->mParent; tempNode; tempNode = tempNode->mParent)
//				{
//					auto NodeTransform = tempNode->mTransformation;
//					GlobalMatrix *= glm::transpose(glm::mat4(NodeTransform.a1, NodeTransform.a2, NodeTransform.a3, NodeTransform.a4,
//						NodeTransform.b1, NodeTransform.b2, NodeTransform.b3, NodeTransform.b4,
//						NodeTransform.c1, NodeTransform.c2, NodeTransform.c3, NodeTransform.c4,
//						NodeTransform.d1, NodeTransform.d2, NodeTransform.d3, NodeTransform.d4));
//				}
//
//				for (std::pair<const double, glm::mat4> p : times)
//				{
//					Assimp::Interpolator<aiVectorKey> vectorInterp;
//					Assimp::Interpolator<aiQuatKey> quatInterp;
//					aiVector3D pos, scale;
//					aiQuaternion rot;
//
//					if (node->mNumPositionKeys > posKey + 1)
//					{
//
//
//						float ratio = 0.0f;
//
//						ratio = (float)((p.first - node->mPositionKeys[posKey].mTime) / (node->mPositionKeys[posKey + 1].mTime - node->mPositionKeys[posKey].mTime));
//
//						vectorInterp(pos, node->mPositionKeys[posKey], node->mPositionKeys[posKey + 1], ratio);
//
//						if (p.first >= node->mPositionKeys[posKey + 1].mTime)
//						{
//							++posKey;
//						}
//					}
//					else
//					{
//						pos = node->mPositionKeys[posKey].mValue;
//					}
//
//					if (node->mNumRotationKeys > rotKey + 1)
//					{
//
//
//						float ratio = 0.0f;
//
//						ratio = (float)((p.first - node->mRotationKeys[rotKey].mTime) / (node->mRotationKeys[rotKey + 1].mTime - node->mRotationKeys[rotKey].mTime));
//
//						quatInterp(rot, node->mRotationKeys[rotKey], node->mRotationKeys[rotKey + 1], ratio);
//						if (p.first >= node->mRotationKeys[rotKey + 1].mTime)
//						{
//							++rotKey;
//						}
//					}
//					else
//					{
//						rot = node->mRotationKeys[rotKey].mValue;
//					}
//
//					if (node->mNumScalingKeys > scaleKey + 1)
//					{
//
//
//						float ratio = 0.0f;
//
//						ratio = (float)((p.first - node->mScalingKeys[scaleKey].mTime) / (node->mScalingKeys[scaleKey + 1].mTime - node->mScalingKeys[scaleKey].mTime));
//
//						vectorInterp(scale, node->mScalingKeys[scaleKey], node->mScalingKeys[scaleKey + 1], ratio);
//						if (p.first >= node->mScalingKeys[scaleKey + 1].mTime)
//						{
//							++scaleKey;
//						}
//					}
//					else
//					{
//						scale = node->mScalingKeys[scaleKey].mValue;
//					}
//
//					auto rotMatrix = rot.GetMatrix();
//					p.second = glm::scale(p.second, glm::vec3(scale.x, scale.y, scale.z));
//					p.second *= glm::transpose(glm::mat4(
//						rotMatrix[0][0], rotMatrix[1][0], rotMatrix[2][0], 0,
//						rotMatrix[0][1], rotMatrix[1][1], rotMatrix[2][1], 0,
//						rotMatrix[0][2], rotMatrix[1][2], rotMatrix[2][2], 0,
//						0, 0, 0, 1
//					));
//					p.second *= glm::translate(glm::mat4(1), glm::vec3(pos.x, pos.y, pos.z));
//
//					aData->frames[index].bonePos[k] = p.second;
//					aData->frames[index].timestampInTicks = p.first;
//					aData->frames[index].timestampInSeconds = (p.first / aData->m_TicksPerSecond);
//
//					++index;
//				}
//			}
//
//			std::vector<aiNode*> stack;
//			stack.push_back(scene->mRootNode);
//			while (!stack.empty())
//			{
//				aiNode* node = stack.back();
//				stack.pop_back();
//
//				if (m->m_BoneNameToIndex.find(node->mName.data) != m->m_BoneNameToIndex.end())
//				{
//					unsigned int index = m->m_BoneNameToIndex[node->mName.data];
//					for (AnimationData::Frame& f : aData->frames)
//					{
//						if (m->m_BoneNameToIndex.find(node->mParent->mName.data) == m->m_BoneNameToIndex.end())
//						{
//							break;
//						}
//						f.bonePos[index] *= f.bonePos[m->m_BoneNameToIndex[node->mParent->mName.data]];
//					}
//
//				}
//				for (unsigned int j = 0; j < node->mNumChildren; ++j)
//				{
//					stack.push_back(node->mChildren[j]);
//				}
//			}
//
//			m->m_Animations[a[i]->mName.C_Str()] = aData;
//		}
//	}
//	return LOADERROR;
//}

//
//std::string DataManager::LoadAnimation(char* path, size_t meshIndex)
//{
//	if (meshIndex >= m_Data.size() || m_Data[meshIndex].m_Type != Data::MeshData)
//	{
//		return "LOADERROR";
//	}
//
//
//
//	return "LOADERROR";
//}
size_t DataManager::LoadTexture(const char* dir)
{
	//threadLock.lock();

	std::string path = GetAssetPath(dir);

	int width, height, channels;
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (data == nullptr)
	{
		//threadLock.unlock();
		return LOADERROR;
	}
	//printf("LOADING TEXTURE : %s", path);
	size_t idx = Renderer::GetInstance()->CreateTexture(dir, data, width * height * 4, width, height);
	free(data);
	//threadLock.unlock();
	return idx;
}
size_t DataManager::LoadTexture(char* dir, uint8_t* file_data, size_t len)
{
	//threadLock.lock();

	std::string path = GetAssetPath(dir);

	int width, height, channels;
	uint8_t* data = stbi_load_from_memory(file_data, (int)len, &width, &height, &channels, 4);
	if (data == nullptr)
	{
		//threadLock.unlock();

		return LOADERROR;
	}
	size_t idx = Renderer::GetInstance()->CreateTexture(path.c_str(), data, width * height * 4, width, height);
	free(data);
	//threadLock.unlock();

	return idx;
}

size_t DataManager::LoadSound(char* dir)
{
	threadLock.lock();

	if (m_CachedId.find(dir) != m_CachedId.end())
	{
		threadLock.unlock();
		return m_CachedId[dir];
	}

	std::string path = GetAssetPath(dir);

	Sound* newSound = new Sound();

	newSound->LoadOGG(path.c_str());

	if (newSound->bufferID == NULL)
	{
		delete newSound;
		threadLock.unlock();

		return LOADERROR;
	}

	Data* newData = new Data();
	newData->m_Data = newSound;
	newData->m_Type = Data::DataType::AudioData;

	m_CachedId[dir] = m_Data.size();
	m_Data.push_back(newData);

	AudioManager::GetInstance()->sounds.push_back(newSound);

	//OggVorbis_File* audio = new OggVorbis_File();
	////FILE* file;
	//if (m_CachedId[dir] != NULL)
	//{
	//	return m_CachedId[dir] - 1;
	//}
	//
	//fopen_s(&file, path, "rb");
	//if (!file)
	//{
	//	printf("ERR: failed to open file %s\n", path);
	//	return LOADERROR;
	//}
	//
	//ov_fopen(path, audio);
	//int retval = ov_open_callbacks(file, audio, nullptr, 0, OV_CALLBACKS_DEFAULT);
	//vorbis_info *vi = ov_info(audio, -1);
	//printf("rate %d channels: %d\n", vi->rate, vi->channels);
	//Data* data = new Data();
	////ov_clear(&audio);
	//data->m_Type = Data::DataType::AudioData;
	//data->m_Data = audio;
	//m_Data.push_back(data);
	//m_CachedId[dir] = m_Data.size();
	threadLock.unlock();

	return m_Data.size() - 1;
}

#ifdef USE_DIRECTX_RENDERER
size_t DataManager::LoadVertexShader(const char* path, ID3D11Device* Device, ID3D11InputLayout*& Layout, const D3D11_INPUT_ELEMENT_DESC* IEdesc, uint16_t numElements)
{
	threadLock.lock();

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	if (IEdesc == nullptr && Layout == nullptr)
	{
		threadLock.unlock();
		return LOADERROR;
	}
	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);

	if (ShaderBytecode != nullptr)
	{

		Data = new ShaderData();
		ShaderData& newData = *Data;

		if (numElements != 0)
		{
			HRESULT hres = Device->CreateInputLayout(IEdesc, numElements, ShaderBytecode, ShaderBytecodeSize, &Layout);
		}

		newData.m_Type = ShaderData::ShaderType::VertexShader;
		Device->CreateVertexShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11VertexShader**)&newData.m_Shader);
		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	threadLock.unlock();

	free(ShaderBytecode);
	return LOADERROR;
}

size_t DataManager::LoadHullShader(const char* path, ID3D11Device* Device)
{
	threadLock.lock();

	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);

	if (ShaderBytecode != nullptr)
	{

		Data = new ShaderData();
		ShaderData& newData = *Data;

		newData.m_Type = ShaderData::ShaderType::HullShader;
		Device->CreateHullShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11HullShader**)&newData.m_Shader);

		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	free(ShaderBytecode);
	threadLock.unlock();

	return LOADERROR;
}

size_t DataManager::LoadDomainShader(const char* path, ID3D11Device* Device)
{
	threadLock.lock();

	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);
	if (ShaderBytecode != nullptr)
	{

		Data = new ShaderData();
		ShaderData& newData = *Data;

		newData.m_Type = ShaderData::ShaderType::DomainShader;
		Device->CreateDomainShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11DomainShader**)&newData.m_Shader);

		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	free(ShaderBytecode);
	threadLock.unlock();

	return LOADERROR;

}

size_t DataManager::LoadGeometryShader(const char* path, ID3D11Device* Device)
{
	threadLock.lock();

	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);
	if (ShaderBytecode != nullptr)
	{

		Data = new ShaderData();
		ShaderData& newData = *Data;

		newData.m_Type = ShaderData::ShaderType::GeometryShader;
		Device->CreateGeometryShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11GeometryShader**)&newData.m_Shader);

		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	free(ShaderBytecode);
	threadLock.unlock();

	return LOADERROR;

}

size_t DataManager::LoadPixelShader(const char* path, ID3D11Device* Device)
{
	threadLock.lock();

	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);

	if (ShaderBytecode != nullptr)
	{

		Data = new ShaderData();
		ShaderData& newData = *Data;

		newData.m_Type = ShaderData::ShaderType::PixelShader;
		Device->CreatePixelShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11PixelShader**)&newData.m_Shader);

		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	free(ShaderBytecode);
	threadLock.unlock();


	return LOADERROR;
}

size_t DataManager::LoadComputeShader(const char* path, ID3D11Device* Device)
{
	threadLock.lock();

	char* ShaderBytecode = nullptr;
	long ShaderBytecodeSize;
	ShaderData* Data = nullptr;

	if (m_CachedShaderId[path] != NULL)
	{
		threadLock.unlock();

		return (m_CachedShaderId[path] - 1);
	}

	Utils::ReadFile(path, ShaderBytecode, ShaderBytecodeSize);

	if (ShaderBytecode != nullptr)
	{
		Data = new ShaderData();
		ShaderData& newData = *Data;

		newData.m_Type = ShaderData::ShaderType::ComputeShader;
		Device->CreateComputeShader(ShaderBytecode, ShaderBytecodeSize, nullptr, (ID3D11ComputeShader**)&newData.m_Shader);

		m_Shaders.push_back(Data);

		m_CachedShaderId[path] = m_Shaders.size();
		threadLock.unlock();

		free(ShaderBytecode);
		return (m_Shaders.size() - 1);
	}
	free(ShaderBytecode);
	threadLock.unlock();

	return LOADERROR;
}
#endif

void* DataManager::Get(size_t index)
{
	if (index >= m_Data.size())
	{
		return nullptr;
	}
	return m_Data[index]->m_Data;
}

void* DataManager::Get(size_t index, Data::DataType expectedType)
{
	if (index > m_Data.size())
	{
		return nullptr;
	}
	if (m_Data[index]->m_Type != expectedType)
	{
		return nullptr;
	}
	else
	{
		return m_Data[index]->m_Data;
	}
}

void* DataManager::GetShader(size_t index)
{
	if (index >= m_Shaders.size())
	{
		return nullptr;
	}
	else
	{
		return m_Shaders[index]->m_Shader;
	}
}

void* DataManager::GetShader(size_t index, ShaderData::ShaderType expectedType)
{
	if (index >= m_Shaders.size())
	{
		return nullptr;
	}
	else if (m_Shaders[index]->m_Type != expectedType)
	{
		return nullptr;
	}
	else
	{
		return m_Shaders[index]->m_Shader;
	}
}

void DataManager::ClearData()
{
	for (Data* Dp : m_Data)
	{
		delete Dp;
		Dp = nullptr;
	}

	m_Data.clear();
	m_CachedId.clear();
}

void DataManager::ClearShaders()
{
	for (ShaderData* Dp : m_Shaders)
	{
		delete Dp;
		Dp = nullptr;
	}

	m_Shaders.clear();
	m_CachedShaderId.clear();
}