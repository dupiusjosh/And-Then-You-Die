#include "Tile.h"
#include "../glm.h"
#include "../DataManager.h"
#include "../Mesh.h"
#include "../MeshComponent.h"

void Tile::Update(float deltaTime)
{
	//Object::Update(deltaTime);
}

void Tile::RegisterContainingRoom(Room* room)
{
	m_ContainingRoom = room;
}

Tile::Tile() : Object()
	{
		m_Type = Invalid;
		m_MeshIndex = -1;
		m_TextureIndex = -1;
		m_DataId = -1;
		m_ContainingRoom = nullptr;
		m_Worth = rand() % (MAXTILEWORTH - MINTILEWORTH);
		m_Worth += MINTILEWORTH;
		//m_VertexBufferOffset = 0;
		//m_IndexBufferOffset = 0;
	}

	Tile::Tile(TileType type, char* path, DataManager* Manager, Room* thing)
	{
		m_Type = type;
		//m_Path = path;
		m_ContainingRoom = thing;
		if (Manager != nullptr)
		{/*m_DataId = Manager->Load(path);*/}
		
	}

	Tile::Tile(XMLTileType _type)
	{
		DataManager* DM = DataManager::GetInstance();
		
		m_MeshIndex = -1;
		m_TextureIndex = -1;
		m_DataId = -1;
		m_ContainingRoom = nullptr;
		m_Worth = rand() % (MAXTILEWORTH - MINTILEWORTH);
		m_Worth += MINTILEWORTH;
		switch (_type)
		{
		case Tile::XMLTileType::XWall:
		{
			m_MeshIndex = DM->LoadMesh("WallTile.fbx", true);
			Mesh* mesh = (Mesh*)DM->Get(m_MeshIndex, Data::DataType::MeshData);
			AddComponent(new MeshComponent(mesh));
			m_Type = Wall;
			tag = Object::Wall;
		} break;
		case Tile::XMLTileType::XFloor:
		{
			m_MeshIndex = DM->LoadMesh("CubeTile.fbx", true);
			Mesh* mesh = (Mesh*)DM->Get(m_MeshIndex, Data::DataType::MeshData);
			AddComponent(new MeshComponent(mesh));
			m_Type = Floor;
			tag = Object::Floor;
		} break;
		default:
			break;
		}
	}

	Tile::~Tile()
	{
		//delete[] m_Path;
	}