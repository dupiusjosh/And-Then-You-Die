#include "GameState.h"
#include "EngineState.h"
#include "Object.h"
#include "DataManager.h"
#include "LevelGeneration\LevelGenerator.h"
#include "Renderer.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "AnimationComponent.h"
#include <glm\glm.hpp>
#include "CollisionDetection.h"
#include "DKEngine.h"
#include "NavMesh.h"
#include "MenuState.h"

#define NETWORK_STUFF 1
void GameState::Load()
{
	printf("<<LOADING GAME STATE\n");
	//using namespace glm;
	DataManager * m_DataManager = DataManager::GetInstance();
	Renderer * m_Renderer = Renderer::GetInstance();

	DKEngine * eng = DKEngine::GetInstance();

#if NETWORK_STUFF
	if (strcmp(m_Data->ip, "") == 0)
	{
		eng->m_Serv = new NetServer();
		eng->m_Serv->INIT(10, 7778);
		eng->m_Serv->DEBUGLOG = true;
	}
#endif

	Canvas* m_Canvas = new Canvas();
	m_ObjManager->AddObject(m_Canvas);

	//printf("Loading sphere.fbx...");
	size_t idx = m_DataManager->LoadMesh("CubeTile.fbx",true);
	Mesh* m = (Mesh*)m_DataManager->Get(idx);
	m->m_Texture = m_DataManager->LoadTexture("BlueFloor.jpg");
	
	//manually set wall texture
	idx = m_DataManager->LoadMesh("WallTile.fbx");
	m = (Mesh*)m_DataManager->Get(idx);
	m->m_Texture = m_DataManager->LoadTexture("walluv-6.jpg");

	//size_t idx;
	idx = m_DataManager->LoadMesh("Enemy-Goblin/Goblin_D_Shareyko.fbx");
	m = (Mesh*)m_DataManager->Get(idx);
	m->m_Texture = 0;
	printf("...Done");
	printf("Loading Sound stuff\n");

	idx = m_DataManager->LoadMesh("WallTile.fbx", true);
	m = (Mesh*)m_DataManager->Get(idx);
	m->m_Texture = m_DataManager->LoadTexture("walluv-6.jpg");

	printf("....Done\n");

	printf("Setting up objects");

	printf("Waiting on Server connection....");

#if NETWORK_STUFF
	eng->m_Client = new NetClient(m_ObjManager, eng->m_inManager, eng->navmesh);
	if(strcmp(m_Data->ip, "") == 0)
		eng->m_Client->Init(7778, "127.0.0.1");// "192.168.0.13");//10.20.33.100 // 192.168.0.15 // 127.0.0.1
	else
		eng->m_Client->Init(7778, m_Data->ip);
	//eng->m_Client->Init(7778, "10.20.33.100");

#if 1
	eng->m_Client->DEBUGLOG = true;
#endif
	while (!eng->m_Client->m_Setup)//isReady())
	{
		//Update
		eng->m_Client->Update();
		//Sleep(1);
	}
#endif

	//m_Renderer->AddLine(glm::vec4(0,0,0,1), glm::vec4(100, 100, 100, 1));

	printf("...Done\n");
	//HACK 
	m_ObjManager->Update(0);

	printf("Partitioning Level...");

	m_ObjManager->BuildQuadTree();
	while (!eng->m_Client->isReady())
	{
		eng->m_Client->Update();
	}
	m_ObjManager->Update(0);

	printf("...Done\n");
	printf("Finished loading\n");
	printf("<<LOADING GAME STATE DONE>>\n");
	FINISH();
}

GameState::GameState(ObjectManager * objMan, PassedData* passedData) //: EngineState::m_ObjManager(objMan)
{
	//loadThread = new std::thread(&GameState::Load, this);
	this->m_Data = passedData;
	m_ObjManager = objMan;
}
void GameState::Update(ObjectManager * om, Renderer * rn, float deltaTime)
{
	//Update for this game state.
	if (!this->isLoaded())
	{

	}
	else
	{
		//actual update
		//this is for win/loss================
		if (updatecnt % 30)
		{
			player = om->GetObjectByTag(Object::Player);
			allems = om->GetObjectsByTag(Object::Enemy);
			if (player)
			{
				StatScript* sc = player->GetComponent<StatScript>();
				if (sc)
				{
					//if (sc->GetCurrentHealth() <= 0.0f)
					//{
					//	this->m_Loaded = false,
					//		DKEngine::GetInstance()->ChangeState(new MenuState(om));
					//}
				}
				if (player->m_Transform[3][1] < -1)
				{
					player->m_Transform[3][1] = 2;
				}
			}

			if (allems.size() == 0)
			{
				/*this->m_Loaded = false;
				DKEngine::GetInstance()->ChangeState(new MenuState(om));*/
			}

		}
		updatecnt++;
		updatecnt++;
		

		//====================================
	}
}
void GameState::StartLoad()
{
	loadThread = new std::thread(&GameState::Load, this);
}

GameState::~GameState()
{
	//is joined in base class
	//while (!isLoaded());
	//loadThread->join();

	auto dk = DKEngine::GetInstance();
	/*delete dk->navmesh;
	dk->navmesh = nullptr;

	delete dk->levelgen;
	dk->levelgen = nullptr;*/
	//delete dk->m_inManager;
	//dk->m_inManager = nullptr;
	delete dk->m_Client;
	dk->m_Client = nullptr;
	delete dk->m_Serv;
	dk->m_Serv = nullptr;
	delete m_Data;
}
