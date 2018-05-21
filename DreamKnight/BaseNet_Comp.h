#pragma once
//#include "NetworkDefines.h"
#include "RAKNET\NetworkIDObject.h"
#include "RAKNET\NetworkIDManager.h"
#include "Component.h"
#include <iostream>
#include "InputManager.h"
class NetClient;
class NetServer;
class InputManager;

class BaseNet_Comp : public Component, public RakNet::NetworkIDObject
{
protected:
	InputManager* m_Input= nullptr;
	bool m_ServSendTransform = false;
	bool m_SyncVelocity = false;
	bool m_isServer=false;
	float m_SyncRate = .3f;
	float elpsTime = 0;

	NetClient * m_Client = nullptr;
	NetServer * m_Server = nullptr;


public:
	enum MessageTypes : uint16_t
	{
		Transform,
		Physics,
		Health,

		CustomMessageStart
	};


	BaseNet_Comp();
	~BaseNet_Comp();


	bool IsServer() { /*printf("%i", (int)m_Server);*/ return m_isServer; }
	void SetIM(InputManager* im) { m_Input = im; }
	void OnUpdate(float deltaTime);
	void ServerUpdate(const float& deltaTime);
	void ClientUpdate(const float& deltaTime);
	virtual void HandleMessage(RakNet::BitStream *bs);
	virtual void PacketUpdate(const float& deltaTime);


	//PURPOSE: Set the rate at which the object syncs in time.
	void SetSyncRate(float f) { m_SyncRate = f; }
	//PURPOSE: If object is on the server, sync the object matrix
	void SetSyncTransform(bool b) { m_ServSendTransform = b; }
	void SetSyncVelocity(bool b) { m_SyncVelocity = b; }

	//PURPOSE: Flags the object as a server object, and set's the server pointer.
	void SetServer(NetServer* ns) { 
		m_isServer = true; 
		m_Server = ns; 
	}

	//PURPOSE: Flags the object as a client object, and sets client pointer.
	void SetClient(NetClient* nc) { 
		m_isServer = false; 
		m_Client = nc; 
		SetSyncTransform(false); 
	}

};

