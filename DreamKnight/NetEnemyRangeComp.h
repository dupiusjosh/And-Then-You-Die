#pragma once
#include "BaseNet_Comp.h"
#include "glm.h"
#include "NavMesh.h"
#include <chrono>
#include "CapsuleCollider.h"
#include "RigidBodyComponent.h"
#include "SoundComponent.h"
#include "StatScript.h"
#include "Projectile.h"

class NetEnemyRangeComp : public BaseNet_Comp
{
public:
	enum MessageTypes : uint16_t
	{
		StateChanged = BaseNet_Comp::MessageTypes::CustomMessageStart,
		ForwardVector,
		CurrentTarget,

		AdditionalMessageStart,
	};
	SoundSourceComponent* m_SndSrc;
	enum class EnemyStates
	{
		Idle,
		Chase,
		Search,
		Attack,
		Dead,
		Stunned
	};

	bool damagedThisFrame = false;

	EnemyStates m_State = EnemyStates::Idle;
private:
	float m_moveSpeed = 3.0f;

	CapsuleCollider * AtkCapsule;

	AnimationComponent* m_animationComponent;
	RigidBodyComponent* m_RigidBody;
	StatScript* m_StatScript = nullptr;
	unsigned int m_CurrPathIndx = 0;
	std::vector<Tile*> m_CurrPath;
	std::vector<Object*> m_Players;

	Object* target = nullptr;
	int tarIndex = 0;

	int m_upCount = 31;
	float tempAtkTimer;
	float atkRange;
	//Distance to player
	float AgroDistance;
	float maxDotResult;
	Navigation::NavMesh * s_nav;// = Navigation::NavMesh::GetInstance();
	Navigation::NavMesh * my_nav = nullptr;
	int tempPath = 0;
	int tempCounter = 0;
	bool navSet = false;
	float timer = 0;
	float timeSinceAttack = 1.0f;

	float m_RepathTimer = 5555;
	float m_CheckTimer = 5555;
	int DropType;
	//Not gonna lie, still think this is probably the best names for this variable.
	//int DropTypeType;
	int DropValue;

	float stunDuration = 0.5f;
	float timeSinceStunned = 0.0f;

	int m_currProj = 0;
	bool m_ISHOT = false;
	std::vector<Projectile*> m_Projectiles;

	Object* m_weapon;
	CapsuleCollider* m_weaponCollider;

	void PacketUpdate(const float& deltaTime);
public:

	//Constructor, basic settings are predef here.
	NetEnemyRangeComp();
	//CLEAR!
	~NetEnemyRangeComp();

	//AI Functionality ===============
	//State machine logic here, very simple impl.
	void ExecuteState(const float& deltaTime);

	void Stun(float deltaTime);

	//Check to see if the enemy can see the player
	//This function works like a cone check, by basically checking to see if the player is within sqr Distance,
	//Then checking if the direction is within the enemies Z range with a dot product result being the deciding factor
	//if the player is behind, or to far, the enemy will not see them.
	bool CheckPlayerVision(const float & deltaTime);

	//Path to the "Target" variable. SoonTM
	//TODO.
	void EnemyPath(const float& deltaTime);

	float timeSinceDamaged = 0.25f;
	float damageColorDuration = 0.25f;
	void DealWithColorShit(float deltaTime);

	//This function should have the enemy attack,
	//and not move until animation is done. (the time for the attack to finish)
	void Attack(const float& deltaTime);

	//TODO: Add die functionality.
	void Die();

	//================================

	void SetMoveSpeed(float f) { m_moveSpeed = f; }
	void OnUpdate(float deltaTime);
	void OnStart();
	void ServerUpdate(const float& deltaTime);
	void ClientUpdate(const float& deltaTime);
	void HandleMessage(RakNet::BitStream *bs);


	EnemyStates GetState();
	void SetState(EnemyStates state);
	void ExitState(NetEnemyRangeComp::EnemyStates newState);
};