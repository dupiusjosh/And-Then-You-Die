#include "LevelGenerator.h"
#include "..\glm.h"
#include "Tile.h"
#include <random>
#include <set>
#include "..\ObjectManager.h"
#include "..\DataManager.h"
#include "..\MeshComponent.h"
#include "..\MeshCollider.h"
#pragma optimize("",off)
LevelGenerator::LevelGenerator(unsigned int seed, ObjectManager* objm)
{
	srand(seed);

	int hall = 0;

	// sets up the tree for rooms to be contained into
	BSPTree * tree = new BSPTree;
	BSPTree::Node * treeRoot = tree->GetRoot();
	m_Length = tree->GetLength();
	m_Width = tree->GetWidth();
	//m_TileGrid = new std::vector<Tile*>;
	(m_TileGrid).resize(m_Width*m_Length);

	std::vector<BSPTree::Node*> vect = tree->GetLeaves();

	for (unsigned int i = 0; i < vect.size(); i++)
	{
		int size = vect[i]->m_Data.m_Length * vect[i]->m_Data.m_Width;
		int sizeL = vect[m_Largest]->m_Data.m_Length * vect[m_Largest]->m_Data.m_Width;
		int sizeS = vect[m_Smallest]->m_Data.m_Length * vect[m_Smallest]->m_Data.m_Width;
		if (sizeS > size)
		{
			m_Smallest = i;
		}
		if (sizeL < size)
		{
			m_Largest = i;
		}
	}

	//Get furthest
	/*unsigned int furthestRoom;
	float fdist = 0;
	for (unsigned int i = 0; i < vect.size(); i++)
	{
		if (i == m_Smallest)
			continue;


	}*/
	// create the rooms
	for (unsigned int i = 0; i < vect.size(); i++)
	{
		Room * room;
		LevelContainer tem;
		tem = vect[i]->m_Data.CreateRoom(vect[i]->m_Data);
		//vect[i]->m_Data = tem;
		if (i == m_Smallest)
		{
			room = new Room(tem.m_Width, tem.m_Length, tem.m_X, tem.m_Y, &m_TileGrid, &m_EnemyPos, &m_PlayerSpawns, m_Width, objm, true);
		}
		else
		{
			room = new Room(tem.m_Width, tem.m_Length, tem.m_X, tem.m_Y, &m_TileGrid, &m_EnemyPos, m_Width, objm, false);
		}
		if (i == m_Largest)
		{
			//bool notSpawned = true;
			//while (notSpawned)
			{
				int x = tem.m_Width/2;
				int y = tem.m_Length/2;
				x += tem.m_X;
				y += tem.m_Y;

				//Spawn chest here
				//auto *temp = m_TileGrid[Access2D(x, y, m_Width)];
				m_ChestSpawn = glm::vec4(x, 0, y, 0);
			}
		}
		delete room;
	}

	std::queue<BSPTree::Node*> cue;
	cue.push(treeRoot);
	LevelContainer lvlcn;

	// create the halls
	std::vector<glm::vec2> centers;
	while (!cue.empty())
	{
		Room * rm = nullptr;
		if (cue.front()->m_Left != nullptr || cue.front()->m_Right != nullptr)
		{
			BSPTree::Node * temp = cue.front();
			cue.pop();
			cue.push(temp->m_Left);
			lvlcn = lvlcn.CreateHall(temp->m_Left->m_Data, temp->m_Right->m_Data);
			rm = new Room(lvlcn.m_Width, lvlcn.m_Length, lvlcn.m_X, lvlcn.m_Y, &m_TileGrid, &m_EnemyPos, m_Width, objm, true);
			cue.push(temp->m_Right);

			centers.push_back(glm::vec2(temp->m_Data.m_XCenter + temp->m_Data.m_X, temp->m_Data.m_YCenter + temp->m_Data.m_Y));
		}
		// HACK
		// im dumb, will fix later - potato
		if (cue.size() == vect.size())
		{
			for (unsigned int i = 0; i < vect.size(); i++)
			{
				cue.pop();
			}
		}
		if (rm != nullptr)
			delete rm;
	}
	delete tree;
	DataManager * dm = DataManager::GetInstance();

	size_t wall = dm->LoadMesh("FinWall.fbx", true);
	size_t ceil = dm->LoadMesh("CubeTile.fbx", true);
	int x;
	int y;

	for (unsigned int i = 0; i < (m_TileGrid).size(); i++)
	{
		x = i % m_Width;
		y = i / m_Width;
		// this is floor and ceiling
		if ((m_TileGrid)[i] != nullptr && !(x == 0 || y == 0 || y == m_Width - 1 || x == m_Length - 1))
		{
			if (m_TileGrid[i]->m_Type == Tile::Prop)
			{
				Tile* temp = new Tile();
				temp->m_MeshIndex = ceil;
				temp->AddComponent(new MeshComponent((Mesh*)dm->Get(temp->m_MeshIndex, Data::DataType::MeshData)));
				temp->m_Y = y;
				temp->m_X = x;
				temp->m_Transform = glm::translate(temp->m_Transform, glm::vec3(x, 0, y));
				objm->AddObject(temp);
				
				m_TileGrid[i]->AddComponent(new MeshCollider((Mesh*)dm->Get(wall)));//new MeshCollider((Mesh*)dm->Get(m_TileGrid[i]->m_MeshIndex, Data::DataType::MeshData)));
				m_TileGrid[i]->m_Transform = glm::translate(glm::mat4(1), glm::vec3(x, 0, y));

			}
			else
			{
				m_TileGrid[i]->AddComponent(new MeshCollider(m_TileGrid[i]->m_MeshComp->GetMesh()));//new MeshCollider((Mesh*)dm->Get(m_TileGrid[i]->m_MeshIndex, Data::DataType::MeshData)));
				m_TileGrid[i]->m_Transform = glm::translate(glm::mat4(1), glm::vec3(x, 0, y));
			}

			m_TileGrid[i]->m_X = x;
			m_TileGrid[i]->m_Y = y;
			objm->AddObject((m_TileGrid)[i]);

			Tile* temp = new Tile();
			temp->m_MeshIndex = ceil;
			temp->AddComponent(new MeshComponent((Mesh*)dm->Get(temp->m_MeshIndex, Data::DataType::MeshData)));
			temp->m_Y = y;
			temp->m_X = x;
			temp->m_Transform = glm::translate(temp->m_Transform, glm::vec3(x, 5.f, y));
			temp->m_Transform = glm::rotate(temp->m_Transform, glm::radians(180.0f), glm::vec3(0, 0, 1));
			objm->AddObject(temp);

		}
		// this is walls
		else
		{
			bool shouldPlace = false;

			if (i % m_Width > 0 && m_TileGrid[i - 1])
				shouldPlace = true;

			if (i % m_Width < m_Width - 1 && m_TileGrid[i + 1])
				shouldPlace = true;
			//printf("%i\n", i);
			if ((int)(i - m_Width) > 0 && m_TileGrid[i - m_Width])
				shouldPlace = true;

			if (i + m_Width < (m_TileGrid).size() && m_TileGrid[i + m_Width])
				shouldPlace = true;

			//if (x == 0 || y == 0 || y == m_Width - 1 || x == m_Length - 1)
			//	shouldPlace = true;

			if (shouldPlace)
			{
				Tile* temp = new Tile();
				temp->m_Transform = glm::translate(temp->m_Transform, glm::vec3(x, 2.5f, y));
				temp->m_Type = Tile::Wall;
				temp->tag = Object::Wall;
				temp->m_MeshIndex = wall;
				temp->AddComponent(new MeshComponent((Mesh*)dm->Get(temp->m_MeshIndex, Data::DataType::MeshData)));
				temp->AddComponent(new MeshCollider((Mesh*)dm->Get(temp->m_MeshIndex, Data::DataType::MeshData)));
				objm->AddObject(temp);
			}
		}
	}

#ifdef _DEBUG
	for (unsigned int y = 0; y < 75; ++y)
	{
		for (unsigned int x = 0; x < 75; ++x)
		{
			bool skip = false;
			for (unsigned int z = 0; z < centers.size(); ++z)
			{
				if (centers[z].x == x && centers[z].y == y)
				{
					skip = true;
					printf("%c", (char)219);

					break;
				}
			}
			if (skip) continue;
			unsigned int i = Access2D(x, y, 75);
			if ((m_TileGrid)[i])
			{
				printf("%c", (char)178);
			}
			else
			{
				printf(" ");
			}
		}
		printf("\n");
	}
#endif
}
#pragma optimize("",on)

LevelGenerator::~LevelGenerator()
{
	//delete[] m_Rooms;
	(m_TileGrid).clear();
	//delete m_TileGrid;
}