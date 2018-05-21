#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include "NavMesh.h"
#include "InputManager.h"
#include "PickupManager.h"

//raknet includes
#include "NetworkDefines.h"
class ServerState;
class LevelGenerator;

class NetServer
{
	//Need access to m_Connection
	friend void PickupManager::SpawnItemPickup(glm::vec3 pos, uint16_t itemId);
	friend void PickupManager::SpawnEffectPickup(glm::vec3 pos, uint16_t itemId);

public:
	struct ClientConnection
	{
		enum ClientConState
		{
			Init=0,
			Starting,
			Ready
		};
		ClientConState m_Status=Init;

		//List of my objects I created
		std::vector<ObjectCreation::NetObjects*> m_CreatedTrack;
		RakNet::SystemAddress sysAdd;
		Object* player = NULL;

	};
private:
	std::mutex m_RunningMutex;
public:
	Navigation::NavMesh * m_NavMesh= nullptr;
	LevelGenerator * levelgen = nullptr;
	unsigned int m_CurrSeed;
	PickupManager* m_Pickups = nullptr;

private:
	std::thread * m_ThdServer;
	RakPeer m_Connection;
	ObjectManager m_ObjManager;

	float m_PackTimeLimit;

	bool m_Running;
	NetworkIDManager m_IdMan;
	//std::vector<ObjectCreation::NetObjects> m_NetObjects;
	std::list<ObjectCreation::NetObjects*> m_NetObjects;
	//std::map<unsigned long, std::vector<ObjectCreation::NetObjects*>> m_CreatedTracker;
public:
	std::map<unsigned long, ClientConnection*> m_ClientConnections;
private:
	std::map<unsigned long, InputManager*> m_InputMap;

	ServerState * m_activeState = nullptr;


	void setRunning(bool v) { m_RunningMutex.lock(); m_Running = v; m_RunningMutex.unlock(); }
	bool isRunning() { m_RunningMutex.lock(); bool v = m_Running; m_RunningMutex.unlock(); return v; }

	void SendToReady(BitStream*, PacketPriority, PacketReliability, char);
	void SendToNear(BitStream*, PacketPriority, PacketReliability, char);

	void PreloadData();
	void Update(float elpsTime);
	void HandlePacket(RakNet::Packet* pack);
	void ClearObjectForHash(unsigned long Hash);
	static void Start(NetServer* serv);
	
public:
	bool DEBUGLOG = false;
	void SendObjPacket(RakNet::BitStream *bs, PacketReliability reliability = UNRELIABLE_SEQUENCED);
	NetServer();
	~NetServer();
	void INIT(unsigned int maxCon, unsigned short port);

	//Creates the object using the CreateObject function in side of network defines, then registers it on the server.
	Object* AddNetworkedObject(ObjectCreation::Types type, unsigned long hash=0);


	//Properties
	ObjectManager* GetObjectManager() { return &m_ObjManager; };
	PickupManager*& GetPickupManager()
	{
		return m_Pickups;
	}
	unsigned int GetSeed() { return m_CurrSeed; }
	void SendSystemMessage(BitStream *message);
};