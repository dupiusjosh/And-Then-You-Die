#pragma once
//#include <winsock2.h>
//#include <winsock.h>


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#pragma comment(lib, "Ws2_32.lib")

#include "RAKNET\WindowsIncludes.h"
//#include "NetworkIDObject.h"
//#include "NetworkIDManager.h"

#include "RAKNET\RakNetTime.h"
#include "RAKNET\NativeFeatureIncludes.h"

#include "RAKNET\RakPeer.h"
#include "RAKNET\RakNetTypes.h"  // MessageID
#include "RAKNET\RakNetStatistics.h" 
#include "RAKNET\NetworkIDObject.h"
#include "RAKNET\NetworkIDManager.h"
#include "RAKNET\BitStream.h"
#include "RAKNET\RakSleep.h"
#include "RAKNET\PacketLogger.h"
#include "RAKNET\MessageIdentifiers.h"

#include "Object.h"
#include "ObjectManager.h"
//#include "Component.h"

#include "BaseNet_Comp.h"
#include "NetworkVenus.h"
#include "NetPlayer.h"
#include "NetEnemy.h"
#include "NetPlayerComponent.h"
#include "Pickup.h"
#include "Chest.h"

#include "MeshComponent.h"
#include "DataManager.h"
#include "RigidBodyComponent.h"
#include "AABBCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"
#include "CollisionDetection.h"
#include "StatScript.h"
#include "StatScriptEvents.h"
using namespace RakNet;
//class BaseNet_Comp : public Component, public RakNet::NetworkIDObject;

class BaseNet_Comp;
enum NetworkMessages : unsigned char
{
	//Idk if I need this
	cGetState = ID_USER_PACKET_ENUM + 1,

	//send client state of game. ENUM
	//Type(4bytes)
	sState,

	//Request list of all objects
	cGetIDObjects,

	//Send list of all objects
	//NumObj(2bytes)|NumID(sizeofNetworkID)|pos(16*4bt float)
	sIDObjects,

	//Client request to register new object
	//Type(2b)|pos(16*4b float)
	cRegisterObject,

	//Server tells clients to register new object
	//Type(2b)|ID(NetworkID long long)|pos(16*4b float)
	sRegisterObject,

	//Server tells client obj is unregistered.
	//sRemoveConnection,ID
	sRemoveObject,

	//Request seed
	cGetSeed,

	//Server sends current seed.
	//int32
	sCurrentSeed,

	//Custom event used in object, defined in my dirived instance of NetworkIDObjects
	//Enables sub enum packets: setObjTransf setObj____
	//These are handled inside of the objects
	//ID(longlong)|DYNAMIC BY OBJECT
	csDynObjectEvent,

	//Client Sync Key event 
	//vkey(char)|state(1bit)
	cKeyEvent,

	//Server Sync Key event
	//systadhash(ulong)|vkey(char)|state(1bit)
	sKeyEvent,

	//Sync mouse pos
	//xPos,yPos
	cMouseUpdate,

	//Sync mouse pos
	//xPos,yPos
	sMouseUpdate,

	//Request the Hash of the system address for this packet
	cGetConnectionHash,

	//Sends Hash to client for this packet
	//unsigned long
	sConnectionHash,

	//tells the server you are ready and setup.
	cReady,

	//New Level Flag
	sNewLevel,

	//All Players dead
	sAllDead,
	//SEED
	COUNTOFMESSAGETYPES

};



namespace ObjectCreation
{
	enum Types : uint16_t
	{
		empty_object,
		Player,
		Enemy,
		PickupsHP,
		PickupsEffect,
		PickupsItem,
		ChestObject,
		ObjectTypeCount
	};

	//Create an object, returns null if object type doesn't exist
	static Object * CreateObject(Types types, NetworkIDManager * idman, NetworkID* id)
	{
		//std::cout << Object::one;
		Object* obj = NULL;
		BaseNet_Comp * comp = nullptr;
		switch (types)
		{
		case empty_object: 
		{
			obj = new Object();
			break;
		}
		break;
		case Player:
		{
			obj = new NetPlayer();
			//((NetPlayerComponent*)comp)->Set(10.0f);
			break;
		}
		case Enemy:
		{
			obj = new NetEnemy();
			break; 
		}
		case PickupsHP: 
		{
			obj = new Pickup();
			((Pickup*)obj)->Init("Capsule.fbx", PickupType::HealthPickup);
			break;
		}
		case PickupsItem: 
		{
			obj = new Pickup();
			((Pickup*)obj)->Init("Capsule.fbx", PickupType::ItemPickup);
			break;
		}
		case PickupsEffect: 
		{
			obj = new Pickup();
			((Pickup*)obj)->Init("Capsule.fbx", PickupType::EffectPickup);
			break;
		}
		case ChestObject: {
			obj = new Chest();
			break;
		}
		case ObjectTypeCount:
		default: 
		{
			//wat
			printf("RECEIVED WEIRD OBJECT: %d\n", types);
			obj = NULL;
			return NULL;
			break; 
		}
	}
	comp = obj->GetComponent<BaseNet_Comp>();

	comp->SetNetworkIDManager(idman);
	if (id != nullptr)
	{
		comp->SetNetworkID(*id);
	}
	//obj->AddComponent(comp);
	return obj;
}

struct NetObjects
{
	ObjectCreation::Types mType;
	NetworkID m_Id;
	uint64_t sysHash;
};
}