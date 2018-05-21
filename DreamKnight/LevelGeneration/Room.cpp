#include "Room.h"
#include "Tile.h"
#include "..\MeshComponent.h"
#include "..\glm.h"
#include <random>
#include "..\tinyxml2\tinyxml2.h"
#include "..\MaterialComponent.h"
#include "..\RigidBodyComponent.h"
#include "..\MeshCollider.h"

#define OutputDebug 0

//#if OutputDebug
#include <sstream>
//#endif

void Room::DoMagic()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("LevelGeneration/Rooms/StartRoom.tmx");
	tinyxml2::XMLElement* eleFont = doc.FirstChildElement();
	SetLength(eleFont->IntAttribute("height"));
	SetWidth(eleFont->IntAttribute("width"));
	eleFont = eleFont->FirstChildElement("layer");
	eleFont = eleFont->FirstChildElement("data");

	std::string stuff = eleFont->GetText();

	int pos = 0;
	std::string tmp;
	std::string del = ",";
	unsigned int w = 0;
	unsigned int l = 0;
	int count = 0;
	m_RoomGrid = new Tile*[m_Width * m_Length];
	ObjectManager * OM = ObjectManager::GetInstance();
	DataManager * DM = DataManager::GetInstance();

	//int meshIdx = DM->LoadMesh("CubeTile.fbx", true);
	//Mesh* mesh = (Mesh*)DM->Get(meshIdx, Data::MeshData);

	while ((pos = stuff.find(del)) != std::string::npos)
	{
		tmp = stuff.substr(0, pos);
		printf("%s", tmp.c_str());
		int t = 0;
		if (tmp.length() != 0)
			t = std::stoi(tmp, 0, 10);
		stuff.erase(0, pos + 1);
		if (t > 0)
		{
			m_RoomGrid[Access2D(w, l, m_Width)] = new Tile((Tile::XMLTileType)t);
			m_RoomGrid[Access2D(w, l, m_Width)]->RegisterContainingRoom(this);
			m_RoomGrid[Access2D(w, l, m_Width)]->m_Transform = glm::translate(glm::mat4(1), glm::vec3(w, 0, -((int)l)));
			OM->AddObject(m_RoomGrid[Access2D(w, l, m_Width)]);
			w += 1;
			if (w >= m_Width)
			{
				w = 0;
				l += 1;
			}
			count++;
		}
	}
	printf("\tcount : %d\n", count);
}

Room::Room(int _w, int _l, int _x, int _z, std::vector<Tile*>* _grid, std::vector<glm::mat4>* _enem, unsigned int _WidthGrid, ObjectManager * objMan, bool _IsHall) : Object()
{
	m_Width = _w;
	m_Length = _l;
	m_X = _x;
	m_Z = _z;
	m_grid = _grid;
	m_EnemyPos = _enem;
	m_WidthGrid = _WidthGrid;
	m_IsHall = _IsHall;
	//ObjectManager * objMan = ObjectManager::GetInstance();
	m_PlayerSpawn = nullptr;
	fillRoom(objMan);
}

Room::Room(int _w, int _l, int _x, int _z, std::vector<Tile*>* _grid, std::vector<glm::mat4>* _enem, std::vector<glm::mat4>* _player, unsigned int _WidthGrid, ObjectManager * objMan, bool _isSpawnRoom) : Object()
{
	m_Width = _w;
	m_Length = _l;
	m_X = _x;
	m_Z = _z;
	m_grid = _grid;
	m_EnemyPos = _enem;
	m_WidthGrid = _WidthGrid;
	//ObjectManager * objMan = ObjectManager::GetInstance();
	m_PlayerSpawn = _player;
	m_SpawnRoom = _isSpawnRoom;
	fillRoom(objMan);
}

Room::Room() : Object()
{
	ObjectManager * objMan = ObjectManager::GetInstance();

	DoMagic();
	//fillRoom(ObjectManager::GetInstance());
	srand(rand());
	m_DoorsUnused = 0;
	m_Value = 0;
}

Room::~Room()
{
	//delete[] m_RoomGrid;
}

void Room::fillRoom(ObjectManager* OM)
{
	//Generate Walls and flat floor
	DataManager* DManager = DataManager::GetInstance();
	//m_RoomGrid = new Tile*[m_Width * m_Length];

	// set up mesh stuff here for faster stuff
	size_t cube = DManager->LoadMesh("CubeTile.fbx", true);
	size_t wall = DManager->LoadMesh("FinWall.fbx", true);
	//std::vector<size_t> props;
	//props.push_back(DManager->LoadMesh("Props/Barrel.fbx", true));
	//props.push_back(DManager->LoadMesh("Props/Chair.fbx", true));
	//props.push_back(DManager->LoadMesh("Props/Cube.fbx", true));

	size_t pillar = (DManager->LoadMesh("Props/Pillar1.fbx", true));
	{
		Mesh* m = (Mesh*)DManager->Get(pillar);
		m->m_Texture = DManager->LoadTexture("Props/Pillar1/Pillar1_low_CHECKERS_Diffuse.jpg");
	}
	int twod;
	int otwod;
	int enemSpawned, enemCount = 0;

	if (m_SpawnRoom == true)
	{
		for (unsigned int i = 0; i < 4; i++)
		{
			glm::mat4 temp;
			if (m_Length < m_Width)
				temp = glm::translate(glm::mat4(1), glm::vec3((m_Width / 2) + m_X + i, 1.25, (m_Length / 2) + m_Z));
			else
				temp = glm::translate(glm::mat4(1), glm::vec3((m_Width / 2) + m_X, 1.25, (m_Length / 2) + m_Z + i));

			(*m_PlayerSpawn).push_back(temp);

			// Prop stuff mess around shit
			//temp[3] += glm::vec4(1, 0, 1,0);
			//Object* prop = new Object;
			//prop->m_Transform = temp;
			//prop->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_prop, Data::DataType::MeshData)));
			//
			//MeshCollider* meshCol = new MeshCollider((Mesh*)DManager->Get(t_prop, Data::DataType::MeshData));
			//prop->AddComponent(meshCol);

			//RigidBodyComponent* rig = new RigidBodyComponent(2.0f, vec3(0.0f, -9.81f, 0.0f), vec3(0), vec3(0), true, true);
			//prop->AddComponent(rig);

		}
	}
	else
	{
		if (m_Width > m_Length)
		{
			enemSpawned = m_Length / 3;
		}
		else
		{
			enemSpawned = m_Width / 3;
		}
	}


	for (unsigned int i = 0; i < m_Length; ++i)
	{
		for (unsigned int j = 0; j < m_Width; ++j)
		{

			twod = Access2D(j, i, m_Width);
			//if (m_SpawnRoom == false && (i + j) % 50 == 0)
			//{
			//	glm::mat4 temp = glm::translate(glm::mat4(1), glm::vec3(j + m_X, 0.5f, ((int)i) + m_Z));
			//	(*m_EnemyPos).push_back(temp);
			//	//Object * enem = new Object;
			//	//enem->tag = Object::Enemy;
			//	//enem->m_Transform = glm::translate(enem->m_Transform, glm::vec3(j + m_X, 2.5f, ((int)i) + m_Z));
			//	//enem->AddComponent(new MaterialComponent(0));
			//	//OM->AddObject(enem);
			//}
			otwod = Access2D((j + m_X), (i + m_Z), m_WidthGrid);
			if (otwod >= (int)m_grid->size())
			{
				break;
			}

			if ((*m_grid)[otwod] && (*m_grid)[otwod]->m_Type == Tile::Prop && !m_IsHall)
				continue;

			Tile * t_Tile = new Tile();
			//m_RoomGrid[twod] = new Tile();
			//m_RoomGrid[twod]->m_ObjManager = OM;
			//m_RoomGrid[twod]->RegisterContainingRoom(this);
			t_Tile->m_ObjManager = OM;
			t_Tile->RegisterContainingRoom(this);
			if ((i == 0 && m_Z == 0) || (m_Z == m_Length - 1 && i == m_Length - 1) || (j == 0 && m_X == 0) || (m_X == m_Width && j == m_Width - 1))
			{
				//m_RoomGrid[twod]->tag = Object::Wall;
				//m_RoomGrid[twod]->m_Type = Tile::Wall;
				t_Tile->tag = Object::Wall;
				t_Tile->m_Type = Tile::Wall;
			}
			else
			{
				//m_RoomGrid[twod]->tag = Object::Floor;
				//m_RoomGrid[twod]->m_Type = Tile::Floor;
				//m_Value += m_RoomGrid[twod]->m_Worth;
				t_Tile->tag = Object::Floor;
				t_Tile->m_Type = Tile::Floor;
			}

			t_Tile->m_Transform = glm::translate(t_Tile->m_Transform, glm::vec3(j + m_X, 0, ((int)i) + m_Z));

			switch (t_Tile->m_Type)
			{
				case (Tile::TileType::Door):
				case (Tile::TileType::Floor):
				{
					//m_RoomGrid[twod]->m_MeshIndex = cube;
					//m_RoomGrid[twod]->AddComponent(new MeshComponent((Mesh*)DManager->Get(m_RoomGrid[twod]->m_MeshIndex, Data::DataType::MeshData)));
					t_Tile->m_MeshIndex = cube;
					//t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));

					break;
				}
				case (Tile::TileType::Wall):
				{
					//m_RoomGrid[twod]->m_MeshIndex = wall;
					//m_RoomGrid[twod]->AddComponent(new MeshComponent((Mesh*)DManager->Get(m_RoomGrid[twod]->m_MeshIndex, Data::DataType::MeshData)));
					//m_RoomGrid[Access2D(j, i, m_Width)]->TextureIndex = DManager->LoadTexture("TileTexture_Floor.png");
					//t_Tile->m_MeshIndex = wall;
					//t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
					break;
				}
				case (Tile::TileType::SlopeSide):
				{
					//m_RoomGrid[Access2D(j, i, m_Width)]->m_MeshIndex = DManager->LoadMesh("SlopeTile.fbx");
					//m_RoomGrid[Access2D(j, i, m_Width)]->TextureIndex = DManager->LoadTexture("TileTexture_Slope.png");
					break;
				}
				case (Tile::TileType::SlopeCornerOuter):
				{
					//m_RoomGrid[Access2D(j, i, m_Width)]->m_MeshIndex = DManager->LoadMesh("SlopeTileCorner.fbx");
					//m_RoomGrid[Access2D(j, i, m_Width - 1)].TextureIndex = DManager->LoadTexture("TileTexture_Slope.png");
					break;
				}
			}

			if ((*m_grid)[otwod] == nullptr)
			{
				t_Tile->m_MeshIndex = cube;
				t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
				(*m_grid)[otwod] = t_Tile;
			}
			else
			{
				if ((*m_grid)[otwod]->m_Type == Tile::Floor && t_Tile->m_Type == Tile::Wall)
				{
					delete t_Tile;
				}
				else if ((*m_grid)[otwod]->m_Type == Tile::Wall && t_Tile->m_Type == Tile::Wall)
				{
					t_Tile->m_Type = Tile::Floor;
					t_Tile->m_MeshIndex = cube;
					t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
					delete (*m_grid)[otwod];
					(*m_grid)[otwod] = t_Tile;
				}
				else if ((*m_grid)[otwod]->m_Type == Tile::Wall && t_Tile->m_Type == Tile::Floor)
				{
					t_Tile->m_Type = Tile::Floor;
					t_Tile->m_MeshIndex = cube;
					t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
					delete (*m_grid)[otwod];
					(*m_grid)[otwod] = t_Tile;
				}
				else
					delete t_Tile;
			}

			//if ((*m_grid)[otwod] != nullptr)
			//{
			//	if ((*m_grid)[otwod]->m_Type == Tile::Floor && t_Tile->m_Type == Tile::Wall)
			//	{
			//		//(*m_grid)[otwod]->m_Type = Tile::Floor;
			//		delete t_Tile;
			//	}
			//	else if ((*m_grid)[otwod]->m_Type == Tile::Wall && t_Tile->m_Type == Tile::Wall)
			//	{
			//		t_Tile->m_Type = Tile::Floor;
			//		t_Tile->m_MeshIndex = cube;
			//		t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
			//		delete (*m_grid)[otwod];
			//		(*m_grid)[otwod] = t_Tile;
			//	}
			//	else if ((*m_grid)[otwod]->m_Type == Tile::Wall && t_Tile->m_Type == Tile::Floor)
			//	{
			//		t_Tile->m_Type = Tile::Floor;
			//		t_Tile->m_MeshIndex = cube;
			//		t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
			//		delete (*m_grid)[otwod];
			//		(*m_grid)[otwod] = t_Tile;
			//	}
			//}
			//else if (t_Tile->m_Type == Tile::Floor)
			//{
			//	t_Tile->m_MeshIndex = cube;
			//	t_Tile->AddComponent(new MeshComponent((Mesh*)DManager->Get(t_Tile->m_MeshIndex, Data::DataType::MeshData)));
			//	(*m_grid)[otwod] = t_Tile;
			//}
			//else
			//{
			//	delete t_Tile;
			//}


		/*	if (enemCount < enemSpawned && !m_SpawnRoom && !m_IsHall && m_RoomGrid[twod]->tag == Object::Floor && (i+j) != 0 &&ran % (i+j) <= 0)
			int ran = rand();
			{
				glm::mat4 temp = glm::translate(glm::mat4(1), glm::vec3(j + m_X, 2.5f, ((int)i) + m_Z));
				(*m_EnemyPos).push_back(temp);
				enemCount++;
			}
			*/
		}
	}

	//Spawn props.
	if (!m_IsHall)
	{
		for (unsigned int i = 0; i < m_Length; ++i)
		{
			for (unsigned int j = 0; j < m_Width; ++j)
			{
				int indx = Access2D((j + m_X), (i + m_Z), m_WidthGrid);
				if ((*m_grid)[indx] != nullptr)
				{
					if ((*m_grid)[indx]->m_Type == Tile::Floor)
					{
						int wallcnt = 0;
						for (unsigned int modx = -1; modx < 2; modx++)
						{
							for (unsigned int mody = -1; mody < 2; mody++)
							{
								if ((j + m_X + modx) < 0 || (j + m_X + modx) > m_WidthGrid)
									continue;
								if ((i + m_Z + mody) < 0 || (i + m_Z + mody) > m_WidthGrid)
									continue;
								int checkindx = Access2D((j + m_X + modx), (i + m_Z + mody), m_WidthGrid);
								if ((*m_grid)[checkindx])
									if ((*m_grid)[checkindx]->m_Type == Tile::Wall || (*m_grid)[checkindx]->m_Type == Tile::Prop)
									{
										wallcnt++;
									}
							}
						}
						if (wallcnt > 3) //Corner
						{
						}
						else if (wallcnt == 0) //centerish room time
						{
							int randval = rand() % 10000;
							if (randval < 100)
							{
								//pilar
								delete (*m_grid)[indx]; //Tile::Prop;
								//(*m_grid)[indx]->m_MeshComp->SetMesh((Mesh*)DManager->Get(pillar));
								Tile * newPillar = new Tile();
								newPillar->m_Static = true;
								newPillar->m_Persistent = false;
								newPillar->m_Transform = glm::translate(glm::mat4(1), glm::vec3((j + m_X), 0, (i + m_Z)));
								newPillar->m_Type = Tile::Prop;
								newPillar->m_MeshIndex = pillar;
								newPillar->AddComponent(new MeshComponent((Mesh*)DManager->Get(pillar)));
								//newPillar->AddComponent(new MeshCollider((Mesh*)DManager->Get(pillar)));
								//m_ObjManager->AddObject(newPillar);
								(*m_grid)[indx] = newPillar;
							}
						}
					}
				}
			}
		}
	}

	if (!m_SpawnRoom && !m_IsHall && (*m_EnemyPos).size() < 40)
	{
		//printf("START ROOM====== %d %d\n", m_Width, m_Length);
		for (/*enemCount*/; enemCount < enemSpawned; enemCount++)
		{
			int x = (rand() % m_Width) + m_X;
			int y = (rand() % m_Length) + m_Z;
			auto * tile = (*m_grid)[Access2D(x, y, m_Width)];
			if (tile && tile->m_Type == Tile::Floor)
			{
				glm::mat4 temp = glm::translate(glm::mat4(1), glm::vec3(x, 0.25f, y));
				(*m_EnemyPos).push_back(temp);
			}
			//printf("%f %f %f\n", temp[3][0], temp[3][1], temp[3][2]);
			//if ((*m_EnemyPos).size() >= 70)
				//break; //:ok_hand:
		}
		//printf("END ROOM======\n");
	}
#if OutputDebug
	//Debug Output of RoomGrid Contents
	std::stringstream consoleOutput;

	char* enumNames[]{ "INVALID ", "None    ", "Floor   ", "Wall    ", "Door    ", "SlopeS  ", "SlopeC  " };

	consoleOutput << "Printing Room Layout:\n";
	for (unsigned int i = 0; i < m_Length; ++i)
	{
		consoleOutput << "\t";
		for (unsigned int j = 0; j < m_Width; ++j)
		{
			consoleOutput << enumNames[m_RoomGrid[Access2D(j, i, m_Width)]->m_Type + 1];
		}
		consoleOutput << "\n";
	}
	consoleOutput << "End Room Layout\n\n";

	printf(consoleOutput.str().data());

#endif

	}

void Room::SetPosition(glm::vec3 & pos)
{
	position = pos;
}

unsigned int Room::GetNumTiles()
{
	return ((m_Width) * (m_Length));
}

unsigned int Room::GetWidth()
{
	return m_Width;
}

unsigned int Room::GetLength()
{
	return m_Length;
}

int Room::getOpenDoors()
{
	return m_DoorsUnused;
}

Tile& Room::GetTile(unsigned int i)
{
	return *m_RoomGrid[i];
}

glm::vec3 & Room::GetPosition()
{
	return position;
}
