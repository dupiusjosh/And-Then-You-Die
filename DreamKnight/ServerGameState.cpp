#include "ServerGameState.h"
#include "NetServer.h"
#include "LevelGeneration\LevelGenerator.h"
#include "Object.h"
#include "NetworkDefines.h"
#include "NetPlayerComponent.h"
#include "NetChestComponent.h"

ServerGameState::ServerGameState()
{
}

void ServerGameState::Start(NetServer * serv)
{
	m_Server = serv;
	elpsTime = 0.0f;
	//The following section needs to be moved when we have propper networked game states working.
	//Generation of the level ================================================================
	//TODO : REMOVE
	/*srand((0));
	serv->m_CurrSeed = rand();*/
	if (serv->levelgen)
		delete serv->levelgen;
	serv->levelgen = new LevelGenerator(serv->GetSeed(), serv->GetObjectManager());
	if (serv->m_NavMesh)
		delete serv->m_NavMesh;
	serv->m_NavMesh = new Navigation::NavMesh();
	serv->m_NavMesh->OnStart(serv->levelgen->GetTileGrid());
	if (serv->m_Pickups) serv->m_Pickups->Reset();
#pragma region Enemy Spawn
	//========================================================
	// Added by Cody, spawns enemies? i think? -Stanko
	//Documented by Ngo 
	size_t size = serv->levelgen->GetEnemys().size();
	//size = 1;
	//printf("ENEMIES: %d\n", size);

	// test player spawn
	//size = levelgen->GetPlayerSpawn().size();
	m_Enemies.reserve(size);
	for (size_t i = 0; i < size; i++)
	{
		//Type of object to create
		ObjectCreation::Types type = ObjectCreation::Types::Enemy;
		//Create the object
		Object * enem = serv->AddNetworkedObject(type);
		//Set the location of the enemies.
		enem->m_Transform = serv->levelgen->GetEnemys()[i];

		//enem->m_Transform = serv->levelgen->GetPlayerSpawn()[0];
		//printf("Enemy Pos: (%f, %f, %f,)\n", enem->m_Transform[3].x, enem->m_Transform[3].y, enem->m_Transform[3].z);
		m_Enemies.push_back(enem);
	}
	//========================================================
#pragma endregion

	

	//Generation of the level end ============================================================
	//Clean up the levelgen
	PickupManager *& Pickups = serv->GetPickupManager();

	if (Pickups == nullptr)
	{
		Pickups = PickupManager::GetInstance();
		Pickups->SetServer(serv);
		Pickups->Init();
	}

	ObjectManager * om = serv->GetObjectManager();
	glm::vec4 chest = serv->levelgen->GetChestSpawn();

	Object* obj = serv->AddNetworkedObject(ObjectCreation::ChestObject);
	obj->m_Transform = glm::translate(glm::mat4(1), glm::vec3(chest.x, chest.y, chest.z));

	obj = serv->AddNetworkedObject(ObjectCreation::ChestObject);
	//Have to append objects using update.
	om->Update(0);
	// Builds QuadTree on the server
	om->BuildQuadTree();
	om->SetNavMesh(serv->m_NavMesh);
}

void ServerGameState::Update(float deltaTime)
{
	//Check for enemys and such.
	elpsTime += deltaTime;
	if (elpsTime > .5f)
	{
		elpsTime = 0;
#pragma region Enemy Alive check, and next level code
		bool allDead = true;
		int alivecnt = 0;
		for (auto e : m_Enemies)
		{
			//if (e->isEnabled()) printf("I am alive. %d", m_Enemies.size());
			auto nec = e->GetComponent<NetEnemyComp>();
			auto nerc = e->GetComponent<NetEnemyRangeComp>();
			if (nec != NULL && nec->GetState() != NetEnemyComp::EnemyStates::Dead)
			{
				allDead = false;
				alivecnt++;
				//break;
			}
			if (nerc != NULL && nerc->GetState() != NetEnemyRangeComp::EnemyStates::Dead)
			{
				allDead = false;
				alivecnt++;
				//break;
			}
		}
		if (allDead)
		{
			//printf("ALL DEADZO\n");
			//need to reload.
			m_Server->m_CurrSeed = rand();

			m_Server->GetObjectManager()->ClearObjects();
			m_Server->GetObjectManager()->Update(0);
			m_Enemies.clear();
			Start(m_Server);
			auto itr = m_Server->m_ClientConnections.begin();
			for (; itr != m_Server->m_ClientConnections.end(); ++itr)
			{
				(*itr).second->m_Status = NetServer::ClientConnection::Starting;
			}
			//move players.
			printf("NEW LEVEL!\n");

			BitStream bsOut;
			bsOut.Write<NetworkMessages>(NetworkMessages::sNewLevel);
			bsOut.Write<unsigned int>(m_Server->m_CurrSeed);
			m_Server->SendSystemMessage(&bsOut);
		}
		else
		{
			//printf("NOT DEADZO %i\n", alivecnt);
		}
#pragma endregion

#pragma region Player dead check, for end game.
		//For now we will kick players to main menu, and close server when all are dead.
		//We will end a game over flag to client's and go from there.
		auto cConns = m_Server->m_ClientConnections.begin();
		allDead = true;
		for (; cConns != m_Server->m_ClientConnections.end(); ++cConns)
		{
			if ((((*cConns).second)->m_Status != NetServer::ClientConnection::Ready))
			{
				allDead = false;
				break;
			}
			if (!((*cConns).second->player))
			{
				allDead = false;
				continue;
			}
			auto stat = (*cConns).second->player->GetComponent<NetPlayerComponent>();

			if (stat->GetState() != PlayerStates::Dead)
			{
				allDead = false;
				break;
			}
		}
		if (allDead && m_Server->m_ClientConnections.size() > 0)
		{
			//RIP
			printf("ALL PLAYERS DEADZO\n");
			BitStream bsOut;
			bsOut.Write<NetworkMessages>(NetworkMessages::sAllDead);
			m_Server->SendSystemMessage(&bsOut);

		}


#pragma endregion

	}
}
