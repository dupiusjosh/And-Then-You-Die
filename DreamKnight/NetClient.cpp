#include <iostream>
#include "NetClient.h"
#include "InputManager.h"
#include "LevelGeneration\LevelGenerator.h"
#include "DKEngine.h"
#include "DeadGameState.h"
NetClient::NetClient(ObjectManager * manager, InputManager* inMan, Navigation::NavMesh * nav)
{
	m_InputManager = inMan;
	//m_InputMap[0] = inMan;
	m_ObjManager = manager;
	m_Navmesh = nav;
}

NetClient::~NetClient()
{
	auto itr = m_InputMap.begin();
	for (; itr != m_InputMap.end(); ++itr)
	{
		//don't delete this one, its deleted in dkengine
		if ((*itr).second == m_InputManager)
			continue;
		delete (*itr).second;
	}
	m_Connection.Shutdown(1, 0, IMMEDIATE_PRIORITY);
}

bool NetClient::isReady()
{
	//gets set once the client receives all the initialization information.
	//The order goes:
	//==Start==
	//ID_CONNECTION_REQUEST_ACCEPTED
	//sConnectionHash
	//cGetSeed
	//sCurrentSeed
	//sRegisterObject
	//==Completed==
	return m_Ready;
}

void NetClient::Init(unsigned short port, std::string ip)
{
	m_Running = true;
	RakNet::SocketDescriptor sd;
	m_Connection.Startup(1, &sd, 1);
	m_Connection.Connect(ip.c_str(), port, 0, 0);
	if (DEBUGLOG)
		std::cout << "CLIENT: Started, trying to connect" << '\n';

	//	myHash = RakNet::SystemAddress::ToInteger(m_Connection.GetMyBoundAddress());
}


void NetClient::Update()
{
	Packet *packet;
	for (packet = m_Connection.Receive(); packet;
		packet = m_Connection.Receive())
	{
		switch (packet->data[0])
		{
#pragma region RakNet HealthChangedEvents


			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			{
				if (DEBUGLOG)
					printf("CLIENT: Another client has disconnected.\n");
				break;
			}
			case ID_REMOTE_CONNECTION_LOST:
			{
				if (DEBUGLOG)
					printf("CLIENT: Another client has lost the connection.\n");
				break;
			}
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
			{
				if (DEBUGLOG)
					printf("CLIENT: Another client has connected.\n");
				break;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				if (DEBUGLOG)
					printf("CLIENT: Our connection request has been accepted.\n");

#if 1 
				// Use a BitStream to write a custom user message
				// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
				RakNet::BitStream bsOut(sizeof(unsigned char));
				bsOut.Write((RakNet::MessageID)cGetConnectionHash);

				m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
#endif DEBUG CODE HERE
				break;
			}
			case ID_NEW_INCOMING_CONNECTION:
			{
				if (DEBUGLOG)
					printf("CLIENT: A connection is incoming.\n");
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				if (DEBUGLOG)
					printf("CLIENT: The server is full.\n");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				if (DEBUGLOG)
					printf("CLIENT: We have been disconnected.\n");
				break;
			}
			case ID_CONNECTION_LOST:
			{
				if (DEBUGLOG)
					printf("CLIENT: Connection lost.\n");
				break;
			}

#pragma endregion
			case cGetState:
			{
				//printf("cGetState\n");
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				if (DEBUGLOG)
					printf("CLIENT: %s\n", rs.C_String());
			}break;
			case sIDObjects:
			{
				//printf("sIDObjects\n");
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//FORMAT: COUNT|TYPE|ID|TYPE|ID____
				uint32_t cnt;
				bsIn.Read(cnt);
				ObjectCreation::Types type;
				NetworkID id;
				//because need meh
				glm::mat4 data(1);
				uint64_t sysHash;
				for (unsigned int i = 0; i < cnt; ++i)
				{
					bsIn.Read<ObjectCreation::Types>(type);
					bsIn.Read<NetworkID>(id);
					bsIn.Read<uint64_t>(sysHash);
					InputManager* inM;
					if (sysHash)
					{
						inM = m_InputMap[(unsigned long)sysHash];
						if (!inM)
						{
							inM = new InputManager();
							m_InputMap[(unsigned long)sysHash] = inM;
						}
					}
					CreateNetworkObject(id, type, data, inM, (unsigned long)sysHash);
				}
				m_Setup = true;

				BitStream bsOut(sizeof(NetworkMessages));
				bsOut.Write(NetworkMessages::cReady);
				m_Connection.Send(&bsOut, MEDIUM_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}break;
			case sRegisterObject:
			{
				//printf("sRegisterObject\n");
				NetworkID id;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				ObjectCreation::Types type;
				//Read type of object to spawn
				bsIn.Read<ObjectCreation::Types>(type);
				glm::mat4 data;
				bsIn.Read<glm::mat4>(data);
				//Read the network ID of object
				bsIn.Read(id);
				uint32_t imID;
				bsIn.Read<uint32_t>(imID);

				
				InputManager *im;// = m_InputMap[imID];
				if (imID != 0)
				{
					im = m_InputMap[imID];
					if (!im)
					{
						im = new InputManager();
						m_InputMap[imID] = im;
					}
					if (DEBUGLOG)
						printf("CLIENT: sysimid:%i\n", (int)imID);

					if (m_Setup)
					{
						if (type == ObjectCreation::Player)
							m_Ready = true;
					}
				}
				CreateNetworkObject(id, type, data, im, imID);
			}break;
			case csDynObjectEvent:
			{
				if (!m_Ready || !m_Setup)
					break;
				//printf("csDynObjectEvent\n");
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				NetworkID id;
				bsIn.Read<NetworkID>(id);
				//printf("%d \n ", id);
				BaseNet_Comp *objComp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(id);
				//bsIn.IgnoreBytes(sizeof(RakNet::NetworkID));
				if (objComp)
				{
					(objComp)->HandleMessage(&bsIn);
				}
				else
				{
					Utils::PrintError("Network object recieved packet, but does not exist.\n");
				}
				
			}break;
			case sKeyEvent:
			{

				if (!m_Ready || !m_Setup)
					break;
				unsigned char vKey;
				bool set;
				uint32_t imID;

				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read<uint32_t>(imID);
				bsIn.Read(vKey);
				bsIn.Read(set);

				if (imID == myHash)
					continue;

				if (m_InputMap[imID] == NULL)
				{
					m_InputMap[imID] = new InputManager();
				}


				if (set)
					m_InputMap[imID]->SetKeyPressed(vKey);
				else
					m_InputMap[imID]->SetKeyUp(vKey);
			}break;
			case sMouseUpdate:
			{
				//printf("sMouseUpdate\n");


				if (!m_Ready || !m_Setup)
					break;
				int xPos, yPos;
				uint32_t imID;

				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(imID);
				bsIn.Read(xPos);
				bsIn.Read(yPos);

				if (imID == myHash)
					continue;
				if (m_InputMap[imID] == NULL)
				{
					m_InputMap[imID] = new InputManager();
				}
				m_InputMap[imID]->SetCurrentMousePos(xPos, yPos);
			}break;
			case sConnectionHash:
			{
				//printf("sConnectionHash\n");
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(myHash);
				m_InputMap[myHash] = m_InputManager;

				BitStream bsOut(sizeof(unsigned char));
				bsOut.Write((RakNet::MessageID)cGetSeed);
				m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}break;
			case sRemoveObject:
			{
				//printf("sRemoveObject\n");

				//printf("Removing object\n");
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				NetworkID hash;
				bsIn.Read<NetworkID>(hash);
				//printf("REMOVING OBJECT: %i", hash);
				BaseNet_Comp * comp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(hash);
				comp->m_GameObject->SetEnabled(false);
			}break;
			case sCurrentSeed:
			{
				//drop the packet if we are ready.
				if (m_Ready)
					break;
				//Get the current seed from server, and build the levels
				unsigned int seed;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read<unsigned int>(seed);
				//Create the level.

				DKEngine * eng = DKEngine::GetInstance();

				eng->levelgen = new LevelGenerator(seed, m_ObjManager);
				//eng->navmesh = Navigation::NavMesh::GetInstance();
				eng->navmesh->OnStart(eng->levelgen->GetTileGrid());

				BitStream bsOut(sizeof(unsigned char));
				bsOut.Write((RakNet::MessageID)cGetIDObjects);
				m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}break;
			case sNewLevel:
			{
				m_Setup = false;
				//printf("NEW LEVEL!\n");
				unsigned int seed;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read<unsigned int>(seed);
				//Create the level.
				m_ObjManager->ClearObjects();
				m_ObjManager->Update(0);
				DKEngine * eng = DKEngine::GetInstance();
				if (eng->levelgen)
				{
					delete eng->levelgen;
					eng->levelgen = nullptr;
				}
				eng->levelgen = new LevelGenerator(seed, m_ObjManager);
				//eng->navmesh = Navigation::NavMesh::GetInstance();
				if (eng->navmesh)
				{
					delete eng->navmesh;
					eng->navmesh = nullptr;
					eng->navmesh = new Navigation::NavMesh();

				}
				m_ObjManager->Update(0);
				m_ObjManager->BuildQuadTree();
				m_ObjManager->SetNavMesh(eng->navmesh);
				eng->navmesh->OnStart(eng->levelgen->GetTileGrid());

				BitStream bsOut(sizeof(unsigned char));
				bsOut.Write((RakNet::MessageID)cGetIDObjects);
				m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);


			}break;
			case sAllDead:
			{
				DKEngine::GetInstance()->ChangeState(new DeadGameState(m_ObjManager));
				break;
			}
			default:
				if (DEBUGLOG)
					printf("CLIENT: Message with identifier %i has arrived.\n", (int)packet->data[0]);
				break;
		}
		m_Connection.DeallocatePacket(packet);
	}

	//send the input state
	//========================
	//if (m_Connection.NumberOfConnections())
	//{
	//	BitStream bsKeyOut;
	//	bsKeyOut.Write<NetworkMessages>(cKeyEvent);

	//	for (unsigned int i = 0; i < (unsigned int)InputManager::KEYMAP::KEYMAPCOUNT; ++i)
	//	{
	//		bool set = m_InputManager->IsKeyPressed((InputManager::KEYMAP)i);
	//		/*if (set)
	//		{
	//			printf("CLIENT: KEY: %i\n", i);
	//		}*/
	//		bsKeyOut.Write(m_InputManager->IsKeyPressed((InputManager::KEYMAP)i));
	//	}
	//	m_Connection.Send(&bsKeyOut, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	//}
	//========================
	if (m_InputMap.size())
		for (auto& val : m_InputMap)
		{
			if (val.first != myHash)
				val.second->Update(0);
		}
}

void  NetClient::CreateNetworkObject(RakNet::NetworkID &id, ObjectCreation::Types type, const glm::mat4 &data, InputManager* im, unsigned long creatorID)
{

	BaseNet_Comp * temp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(id);
	if (temp)
	{
		if (DEBUGLOG)
			printf("CLIENT: Object already exists: %i\n", (int)id);
	}
	else
	{
		//printf("CREATE OBJECT: %i", (int)type);
		Object *obj = ObjectCreation::CreateObject(type, &m_IdMan, &id);
		obj->m_Transform = data;
		obj->m_Creator = creatorID;
		obj->m_MyHash = &myHash;
		if (DEBUGLOG)
			printf("CLIENT: type:%i id: %i\n", (int)type, (int)id);
		BaseNet_Comp *comp = obj->GetComponent<BaseNet_Comp>();
		comp->SetNetworkID(id);
		comp->SetClient(this);

		id = comp->GetNetworkID();

		//obj->AddComponent(comp);
		temp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(id);
		m_ObjManager->AddObject(obj);
		if (DEBUGLOG)
		{
			printf("CLIENT: type:%i id: %i match: %i IMADD:%p\n", (int)type, (int)id, (int)(temp == comp), im);
		}

		comp->SetIM(im);

	}
}

void NetClient::UpdateKeyValue(unsigned char vKey, bool set)
{
	RakNet::BitStream bsOut(20);
	bsOut.Write((RakNet::MessageID)cKeyEvent);
	bsOut.Write(vKey);
	bsOut.Write(set);
	m_Connection.Send(&bsOut, MEDIUM_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void NetClient::SendObjPacket(RakNet::BitStream * bs)
{
	RakNet::BitStream newBs;
	newBs.Write((RakNet::MessageID)NetworkMessages::csDynObjectEvent);
	newBs.Write(bs, bs->GetNumberOfBitsUsed());
	m_Connection.Send(&newBs, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void NetClient::SetCurrentMousePos(int xPos, int yPos)
{
	RakNet::BitStream bsOut(12);
	bsOut.Write((RakNet::MessageID)cMouseUpdate);
	bsOut.Write<int>(xPos);
	bsOut.Write<int>(yPos);
	m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

//Should never call this...RIP... all objects should be created on server first.
void NetClient::CreateObject(ObjectCreation::Types type, glm::mat4 mat)
{

	RakNet::BitStream bsOut(600);
	bsOut.Write((RakNet::MessageID)cRegisterObject);
	bsOut.Write<ObjectCreation::Types>(type);
	bsOut.Write<glm::mat4>(mat);
	m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}
