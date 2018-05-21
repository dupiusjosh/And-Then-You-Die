#pragma once
#include "NetworkDefines.h"
#include <string>
#include <map>
#include "NavMesh.h"
class InputManager;

class NetClient
{
	bool m_Running = false;
	bool m_Ready = false;
	RakPeer m_Connection;
	NetworkIDManager m_IdMan; 
	ObjectManager* m_ObjManager;
	InputManager* m_InputManager;
	Navigation::NavMesh * m_Navmesh;
	std::map<unsigned long, InputManager*> m_InputMap;
public:
	unsigned long myHash;
	bool DEBUGLOG = false;
	//bool m_Ready = false;

	NetClient(ObjectManager * manager, InputManager * inMan, Navigation::NavMesh * nav);
	~NetClient();

	bool isReady();
	bool m_Setup = false;

	void Init(unsigned short port,std::string ip);

	void Update();

	void CreateNetworkObject(RakNet::NetworkID &id, ObjectCreation::Types type, const glm::mat4 &data, InputManager* im, unsigned long creatorID);
	
	void CreateObject(ObjectCreation::Types type, glm::mat4 mat);
	void UpdateKeyValue(unsigned char vKey, bool set);

	void SendObjPacket(RakNet::BitStream *bs);

	void SetCurrentMousePos(int xPos, int yPos);

	
};

