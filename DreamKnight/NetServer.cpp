#include "NetServer.h"
#include "LevelGeneration\LevelGenerator.h"
#include <iostream>
#include "CollisionDetection.h"
#include "PickupManager.h"
#include "ServerGameState.h"
#include "ServerState.h"

void NetServer::SendToReady(BitStream * bs, PacketPriority prio, PacketReliability rel, char chan)
{
	auto itr = m_ClientConnections.begin();
	for (; itr != m_ClientConnections.end(); ++itr)
	{
		if ((*itr).second->m_Status == ClientConnection::Ready)
		{
			m_Connection.Send(bs, prio, rel, chan, (*itr).second->sysAdd, false);
		}
	}
}

void NetServer::SendToNear(BitStream *, PacketPriority, PacketReliability, char)
{
	//TODO ADD
}

void NetServer::PreloadData()
{
	//Preload a player.
	NetPlayer * temp = new NetPlayer();
	m_ObjManager.AddObject(temp);
	//Kappa
	m_ObjManager.Update(0);
	m_ObjManager.ClearObjects(true);
	m_ObjManager.Update(0);
}

void NetServer::Update(float elpsTime)
{
	m_ObjManager.Update(elpsTime);
	for (auto i : m_InputMap)
	{
		i.second->Update(elpsTime);
	}
}
void NetServer::HandlePacket(RakNet::Packet* packet)
{
	switch (packet->data[0])
	{
#pragma region RakNet HealthChangedEvents


		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
		{
			if (DEBUGLOG)
				printf("SERVER: Another client has disconnected: %i.\n", RakNet::SystemAddress::ToInteger(packet->systemAddress));
			break;
		}
		case ID_REMOTE_CONNECTION_LOST:
		{
			if (DEBUGLOG)
				printf("ERVER: Another client has lost the connection: %i.\n", RakNet::SystemAddress::ToInteger(packet->systemAddress));
			break;
		}
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
		{
			if (DEBUGLOG)
				printf("SERVER: Another client has connected.\n");
			break;
		}
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			//should only happen on client
			if (DEBUGLOG)
				printf("SERVER: Our connection request has been accepted.\n");

			// Use a BitStream to write a custom user message
			// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
			//RakNet::BitStream bsOut;
			//bsOut.Write((RakNet::MessageID)cGetState);
			//bsOut.Write("Hello world");
			//m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			break;
		}
		case ID_NEW_INCOMING_CONNECTION:
		{
			if (DEBUGLOG)
				printf("SERVER: A connection is incoming.\n");
			ClientConnection * conn = new ClientConnection();
			conn->sysAdd = packet->systemAddress;
			m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)] = conn;
			break;
		}
		case ID_NO_FREE_INCOMING_CONNECTIONS:
		{
			if (DEBUGLOG)
				printf("SERVER: The server is full.\n");
			break;
		}
		case ID_DISCONNECTION_NOTIFICATION:
		{
			//This is fired when a client calls shutdown.
			if (DEBUGLOG)
				printf("SERVER: We have been disconnected : %i\n", RakNet::SystemAddress::ToInteger(packet->systemAddress));
			//ClearObjectForHash(RakNet::SystemAddress::ToInteger(packet->systemAddress));

			//should destroy their objects...
			//Gonna have to change a lot of things to get this feature.
			break;
		}
		case ID_CONNECTION_LOST:
		{
			//This is called when a client closes ungracefully
			if (DEBUGLOG)
				printf("SERVER: Connection lost. %i\n", RakNet::SystemAddress::ToInteger(packet->systemAddress));
			//ClearObjectForHash(RakNet::SystemAddress::ToInteger(packet->systemAddress));
			break;
		}

#pragma endregion

		case cGetState:
		{
			//RakNet::RakString rs;
			//RakNet::BitStream bsIn(packet->data, packet->length, false);
			//bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			//bsIn.Read(rs);

			//if (DEBUGLOG)
				//printf("SERVER: %s\n", rs.C_String());


			//Need to get curr/ent items in the scene
			RakNet::BitStream bsOut;
			//bsOut.Write<NetworkMessages>(NetworkMessages::sState);
			//bsOut.Write("NEED TO BE ADDED");
			//m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

			break;
		}
		case cGetIDObjects:
		{
			RakNet::BitStream bsOut;
			bsOut.Write<NetworkMessages>(NetworkMessages::sIDObjects);
			//FORMAT: COUNT|TYPE|ID|TYPE|ID_______________
			uint32_t size = (uint32_t)m_NetObjects.size();
			bsOut.Write<uint32_t>(size);
			//Switched from a vector to a list.
			/*for (size_t i = 0; i < size; ++i)
			{
				bsOut.Write<ObjectCreation::Types>(m_NetObjects[i].mType);
				bsOut.Write<NetworkID>(m_NetObjects[i].m_Id);
				bsOut.Write(m_NetObjects[i].sysHash);
			}*/
			for (auto itr = m_NetObjects.begin(); itr != m_NetObjects.end(); ++itr)
			{
				bsOut.Write<ObjectCreation::Types>((*itr)->mType);
				bsOut.Write<NetworkID>((*itr)->m_Id);
				bsOut.Write<uint64_t>((*itr)->sysHash);
			}
			//for (auto itr = m_ClientConnections.begin(); itr != m_ClientConnections.end(); ++itr)
			//{
			//	for (auto itr2 = (*itr).second->m_CreatedTrack.begin(); itr2 != (*itr).second->m_CreatedTrack.end(); ++itr2){
			//		bsOut.Write<ObjectCreation::Types>((*itr2)->mType);
			//		bsOut.Write<NetworkID>((*itr2)->m_Id);
			//		bsOut.Write<uint64_t>((*itr2)->sysHash);
			//	}
			//}
			m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			break;
		}
		case cRegisterObject:
		{

			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

			//Object::Types type;
			ObjectCreation::Types type;

			glm::mat4 data;
			bsIn.Read < ObjectCreation::Types >(type);
			bsIn.Read<glm::mat4x4>(data);
			Object * obj = AddNetworkedObject(type, RakNet::SystemAddress::ToInteger(packet->systemAddress));//ObjectCreation::CreateObject(type, &m_IdMan, NULL);
			obj->m_Transform = data;
			BaseNet_Comp * comp = obj->GetComponent<BaseNet_Comp>();
			NetworkID id = comp->GetNetworkID();

			if (m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] == NULL)
			{
				m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] = new InputManager();
			}
			comp->SetIM(m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)]);

			RakNet::BitStream bsOut;
			bsOut.Write((RakNet::MessageID)sRegisterObject);
			bsOut.Write<ObjectCreation::Types>(type);
			bsOut.Write<glm::mat4>(data);
			bsOut.Write<NetworkID>(id);
			bsOut.Write<uint32_t>((uint32_t)RakNet::SystemAddress::ToInteger(packet->systemAddress));
			m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
		case csDynObjectEvent:
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			NetworkID id;
			bsIn.Read<NetworkID>(id);
			BaseNet_Comp *objComp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(id);
			//bsIn.IgnoreBytes(sizeof(RakNet::NetworkID));
			if (objComp)
			{
				(objComp)->HandleMessage(&bsIn);
			}
			break;
		}
		case cKeyEvent:
		{
			//format VK_Keyvalue|state
			unsigned char vKey;
			bool set;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(NetworkMessages));
			bsIn.Read(vKey);
			bsIn.Read(set);
			//m_Connection.Send(&bsOut, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			//Send to networked input manager
			if (m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] == NULL)
			{
				m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] = new InputManager();
			}

			if (set)
				m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->SetKeyPressed(vKey);
			else
				m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->SetKeyUp(vKey);


			//InputManager * im = m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)];
			RakNet::BitStream bsOut;
			bsOut.Write<NetworkMessages>(sKeyEvent);
			bsOut.Write<uint32_t>((uint32_t)RakNet::SystemAddress::ToInteger(packet->systemAddress));
			bsOut.Write(vKey);
			bsOut.Write(set);

			//for (unsigned int i = 0; i < (unsigned int)InputManager::KEYMAP::KEYMAPCOUNT; ++i)
			//{
			//	bsIn.Read(set);
			//	if (set)
			//		im->SetKeyPressed((InputManager::KEYMAP)i);
			//	else
			//		im->SetKeyUp((InputManager::KEYMAP)i);
			//	bsOut.Write(set);
			//	/*if (set)
			//	{
			//		if (set)
			//			printf("SERVER: KEY: %i\n", i);
			//	}*/
			//}
			//packet->systemAddress
			m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

			break;
		}
		case cMouseUpdate:
		{
			int xPos, yPos;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(NetworkMessages));
			bsIn.Read(xPos);
			bsIn.Read(yPos);
			//m_Connection.Send(&bsOut, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			//Send to networked input manager
			if (m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] == NULL)
			{
				m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] = new InputManager();
			}

			m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->SetCurrentMousePos(xPos, yPos);

			RakNet::BitStream bsOut;
			bsOut.Write<NetworkMessages>(sKeyEvent);
			bsOut.Write<uint32_t>((uint32_t)RakNet::SystemAddress::ToInteger(packet->systemAddress));
			bsOut.Write(xPos);
			bsOut.Write(yPos);
			m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
		case cGetConnectionHash:
		{

			RakNet::BitStream bsOut;
			bsOut.Write<NetworkMessages>(sConnectionHash);
			bsOut.Write<uint32_t>((uint32_t)RakNet::SystemAddress::ToInteger(packet->systemAddress));
			//printf("SERVER GET HASH: %i", RakNet::SystemAddress::ToInteger(packet->systemAddress));
			m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false);
			break;
		}
		case cGetSeed:
		{
			BitStream bs;
			bs.Write<NetworkMessages>(sCurrentSeed);
			bs.Write<unsigned int>(m_CurrSeed);
			m_Connection.Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
		}break;
		case cReady:
		{
			//literally just changing the state of the client
			if (m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->m_Status == ClientConnection::Ready) 
				break;
			if (m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->player)
			{
				std::vector<glm::mat4> spawnloc = levelgen->GetPlayerSpawn();
				auto temp = spawnloc[rand() % spawnloc.size()];
				//printf("NEW PLAYER POS: %f %f %f\n", temp[3][0], temp[3][1], temp[3][2]);
				//spawn at a random loc
				auto * npc = m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->player->GetComponent<NetPlayerComponent>();
				m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->m_Status = ClientConnection::Ready;
				if (npc->GetState() == PlayerStates::Dead)
				{

					//hes dead, rez, and give hp
					npc->SetState(PlayerStates::Idle);
					auto * sc = npc->m_GameObject->GetComponent<StatScript>();
					sc->SetCurrentHealth(sc->GetMaxHealth() * .5f);
					
				}
				m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->player->m_Transform = temp;
				//temp = m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->player->m_Transform;
				//printf("NEW PLAYER POS: %f %f %f\n", temp[3][0], temp[3][1], temp[3][2]);

			}
			else
			{
				m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->m_Status = ClientConnection::Ready;
				//std::chrono::time_point<std::chrono::system_clock> start;
				//start = std::chrono::system_clock::now();

				//Spawn their player.
				Object * obj = AddNetworkedObject(ObjectCreation::Types::Player, RakNet::SystemAddress::ToInteger(packet->systemAddress));//ObjectCreation::CreateObject(type, &m_IdMan, NULL);
				std::vector<glm::mat4> spawnloc = levelgen->GetPlayerSpawn();
				//spawn at a random loc
				obj->m_Transform = spawnloc[rand() % spawnloc.size()];
				m_ClientConnections[RakNet::SystemAddress::ToInteger(packet->systemAddress)]->player = obj;
				BaseNet_Comp * comp = obj->GetComponent<BaseNet_Comp>();
				NetworkID id = comp->GetNetworkID();

				if (m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] == NULL)
				{
					m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)] = new InputManager();
				}
				comp->SetIM(m_InputMap[RakNet::SystemAddress::ToInteger(packet->systemAddress)]);

				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)sRegisterObject);
				bsOut.Write(ObjectCreation::Types::Player);
				bsOut.Write<glm::mat4>(obj->m_Transform);
				bsOut.Write<NetworkID>(id);
				bsOut.Write<uint32_t>((uint32_t)RakNet::SystemAddress::ToInteger(packet->systemAddress));
				m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				//float dTime = (float)(std::chrono::system_clock::now() - start).count() / 1e7f;
				//printf("Delta of creation: %f\n", dTime);
			}
			break;
		}
		default:
			if (DEBUGLOG)
				printf("SERVER: Message with identifier %i has arrived.\n", (int)packet->data[0]);
			break;
	}
}

void NetServer::ClearObjectForHash(unsigned long Hash)
{
	printf("finding objects\n");
	if (m_ClientConnections.find(Hash) != m_ClientConnections.end())
	{
		printf("Clearing objects\n");

		std::vector<ObjectCreation::NetObjects*>& vec = m_ClientConnections[Hash]->m_CreatedTrack;

		for (size_t i = 0; i < vec.size(); ++i)
		{
			//printf("Removing this: %i\n", vec[i]->m_Id);
			BaseNet_Comp * comp = m_IdMan.GET_OBJECT_FROM_ID<BaseNet_Comp*>(vec[i]->m_Id);
			comp->m_GameObject->SetEnabled(false);
			BitStream bs;
			bs.Write<NetworkMessages>(sRemoveObject);
			bs.Write<NetworkID>(vec[i]->m_Id);
			m_Connection.Send(&bs, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			m_NetObjects.remove(vec[i]);

			delete vec[i];
		}
		vec.clear();
	}
}

//Thread entry point, used to Run the server loop
void NetServer::Start(NetServer* serv)
{
	//serv->m_PackTimeLimit = .000001f;
	RakPeer * activeServer = &serv->m_Connection;

	if (serv->DEBUGLOG)
		std::cout << "SERVER: RUNNING" << '\n';
	//Need to preload data.
	serv->PreloadData();

	//Variables used for FPS
	float fpsUpdate = 2;
	float time = 0;
	int counter = 0;

	serv->m_activeState = new ServerGameState();
	serv->m_activeState->Start(serv);

	//auto CodyWillHateThisName = serv->AddNetworkedObject(ObjectCreation::ChestObject, 0);
	//CodyWillHateThisName->m_Transform = glm::mat4(1);

	std::chrono::time_point<std::chrono::system_clock> start;
	start = std::chrono::system_clock::now();

	RakNet::Packet* packet;
	float dTime = 0;
	float elpsTime = 0;

	while (serv->isRunning())
	{
		//Packet holder variable
		//Delta Time

		dTime = (float)(std::chrono::system_clock::now() - start).count() / 1e7f;

		serv->m_activeState->Update(dTime);
		//add back later.
		//if (dTime > 1 / 40.0f && serv->DEBUGLOG) 
			//printf("DTIME BAD: %f\n", dTime);


		//Start time of listening for packets
		start = std::chrono::system_clock::now();

		//Time we have been listening for packets
		//Loop until time limit is reached

		//std::chrono::time_point<std::chrono::system_clock> packetStart;
		//packetStart = std::chrono::system_clock::now();
		elpsTime = 0;
		
		while (elpsTime < serv->m_PackTimeLimit)
		{
			//Get the next packet in the system. 
			for (packet = activeServer->Receive();
				// check if packet is not null, or if we are over time.
				packet;// && dTime < serv->m_PackTimeLimit;//+1; //Removed the time limit, as its borked.
				//to iterate, remove prior packet, then receive new one
				activeServer->DeallocatePacket(packet), packet = activeServer->Receive())
			{
				//Pass packet to the packet handler.
				serv->HandlePacket(packet);
				//dTime += 1;
				//elpsTime = (float)(std::chrono::system_clock::now() - start).count() / 1e7f;
				//if (elpsTime > 1.0f / 60.0f) 
				//	break;
			}
			//How long we have been in the while loop
			elpsTime = (float)(std::chrono::system_clock::now() - start).count() / 1e7f;
		}

		//If the server is in debug mode, execture updates per second logic.
		if (serv->DEBUGLOG)
		{
			//Time since last fps print
			time += dTime;
			//Check if it is time to update fps
			if (fpsUpdate < time)
			{
				//shitty way to do timer
				fpsUpdate = time + 2;
				//print and reset the update counter
				printf("SERVER: %i updates per second\n", counter / 2);
				counter = 0;
			}
			//increment the update counter
			++counter;
		}
		//Update the server
		serv->Update(dTime);
	}
}

void NetServer::SendObjPacket(RakNet::BitStream * bs, PacketReliability reliability)
{
	if (m_Connection.NumberOfConnections())
	{
		RakNet::BitStream newBs;
		newBs.Write((RakNet::MessageID)NetworkMessages::csDynObjectEvent);
		newBs.Write(bs, bs->GetNumberOfBitsUsed());
		//m_Connection.Send(&newBs, MEDIUM_PRIORITY, reliability, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		SendToReady(&newBs, MEDIUM_PRIORITY, reliability, 0);
	}
}

NetServer::NetServer()
{
	//setRunning(true);
}

NetServer::~NetServer()
{
	if (m_ThdServer)
	{
		setRunning(false);
		m_ThdServer->join();
	}
	m_Connection.Shutdown(1, 0, IMMEDIATE_PRIORITY);
	//Clean up input maps
	auto itr = m_InputMap.begin();
	for (; itr != m_InputMap.end(); ++itr)
	{
		delete (*itr).second;
	}
	//Clean up connections
	auto itr2 = m_ClientConnections.begin();
	for (; itr2 != m_ClientConnections.end(); ++itr2)
	{
		delete (*itr2).second;
	}

	//Clean up NetObjects
	for (auto itr3 : m_NetObjects)
	{
		delete itr3;
	}
	m_NetObjects.clear();

	m_ObjManager.ClearObjects(true);
	m_ObjManager.Update(0);
	delete m_ThdServer;
	m_ThdServer = nullptr;
	delete levelgen;
	levelgen = nullptr;
	delete m_NavMesh;
	m_NavMesh = nullptr;
	delete m_Pickups;
	m_Pickups = nullptr;
	m_IdMan.Clear();
}

void NetServer::INIT(unsigned int maxCon, unsigned short port)
{
	SocketDescriptor sd(port, 0);
	m_Connection.Startup(maxCon, &sd, 1);

	this->m_PackTimeLimit = .00000000001f;//1.0f / 60.0f;//
	//m_NavMesh = new Navigation::NavMesh();
	m_ObjManager.SetNavMesh(m_NavMesh);

	//Generate seed for server random based on time.
	srand((unsigned int)time(NULL));
	m_CurrSeed = rand();
	//==============================================

	setRunning(true);

	if (DEBUGLOG)
		std::cout << "SERVER: STARTING" << '\n';
	m_Connection.SetMaximumIncomingConnections(maxCon);
	m_ThdServer = new std::thread(Start, this);

}

Object * NetServer::AddNetworkedObject(ObjectCreation::Types type, unsigned long hash)
{
	Object * obj = ObjectCreation::CreateObject(type, &m_IdMan, NULL);
	//printf("Making object return: %i\n", (obj);
	//Get the network component
	BaseNet_Comp * comp = obj->GetComponent<BaseNet_Comp>();

	//Set the server reference
	comp->SetServer(this);

	//Add object to the server Object Manager
	m_ObjManager.AddObject(obj);
	//Add object to list of networked objects, this is used to sync networked objects between active sessions 
	//( mostly new sessions is this called)
	ObjectCreation::NetObjects *netobj = new ObjectCreation::NetObjects;
	//Type of the object
	netobj->mType = type;

	//Network ID of the object
	//Get the network ID of the object
	NetworkID id = comp->GetNetworkID();
	netobj->m_Id = id;

	//Object creator's system hash, 0 because it wasn't created by a client.
	netobj->sysHash = hash;
	//Append
	m_NetObjects.push_back(netobj);
	if (hash != 0)
	{
		m_ClientConnections[hash]->m_CreatedTrack.push_back(netobj);
	}
	return obj;
}

void NetServer::SendSystemMessage(BitStream *message)
{
	//send bit message to all.
	m_Connection.Send(message, HIGH_PRIORITY, RELIABLE_ORDERED, 2, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}
