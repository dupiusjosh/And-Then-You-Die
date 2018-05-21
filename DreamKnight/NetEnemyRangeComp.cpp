#include "NetEnemyRangeComp.h"

#include "glm.h"
#include <Random>
#include "NetClient.h"
#include "NetServer.h"
#include "MaterialComponent.h"
#include "StatScriptEvents.h"
#include "NetPlayerComponent.h"
#include "AnimationComponent.h"
#include "PickupManager.h"
#include "ItemRef.h"
#include "TimedEffects.h"
#include "HitManagerComponent.h"

#define IdleAnimation "idle"
#define WalkAnimation "walk"
#define AttackAnimation "punch"
#define StunAnimation "stun"
#define DeathAnimation "death"

namespace EnemyAttackRange
{
	int atkcnt = 0;
	void WeaponCollisionEnter(CollisionEvent e)
	{

		if (e.m_CollidingWith->m_GameObject->tag == Object::Wall)
		{
			e.m_ThisObject->m_GameObject->SetEnabled(false);
			return;
		}

		if (e.m_CollidingWith->GetColliderTag() != ColliderTag::HurtBox)
			return;

		if (e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Player)
		{
			if (e.m_ThisObject->m_GameObject->GetComponent<HitManager>()->RegisterHit(e.m_CollidingWith->m_GameObject))
			{

				Object* player = e.m_CollidingWith->m_GameObject;

				float Damage = ((Projectile*)e.m_ThisObject->m_GameObject)->GetDamage();

				auto Stats = player->GetComponent<StatScript>();
				if (Stats)
				{
					Stats->TakeDamage(Damage, e.m_ThisObject->m_GameObject);
				}
				//printf("ENTERED OBJECT\n");
				//printf("HIT : %i atkCnt: %i  SERVER: %i\n", e.m_ThisObject->m_GameObject, atkcnt,e.m_CollidingWith->m_GameObject->GetComponent<BaseNet_Comp>()->IsServer());
			}
		}
	}

	void WeaponCollisionStay(CollisionEvent e)
	{
	}

	void WeaponCollisionExit(CollisionEvent e)
	{
	}

}

void NetEnemyRangeComp::PacketUpdate(const float & deltaTime)
{
	//BaseNet_Comp::PacketUpdate(deltaTime);

	// add custom messages here.
	if (m_isServer)
	{
		elpsTime += deltaTime;
		if (this->m_SyncRate < elpsTime)
		{
			if (m_ServSendTransform)
			{
				RakNet::BitStream bs;
				bs.Write(GetNetworkID());
				bs.Write(BaseNet_Comp::MessageTypes::Transform);
				bs.Write<glm::mat4x4>(m_GameObject->m_Transform);
				m_Server->SendObjPacket(&bs);
			}
			{
				RakNet::BitStream bs;
				bs.Write(GetNetworkID());
				bs.Write(BaseNet_Comp::MessageTypes::Health);
				bs.Write<float>(m_StatScript->GetCurrentHealth());
				m_Server->SendObjPacket(&bs);
			}
			//{
			//	RakNet::BitStream bs;
			//	bs.Write(GetNetworkID());
			//	bs.Write(MessageTypes::StateChanged);
			//	bs.Write<EnemyStates>(m_State);
			//	m_Server->SendObjPacket(&bs);
			//}
			{
				RakNet::BitStream bs;
				bs.Write(GetNetworkID());
				bs.Write(MessageTypes::CurrentTarget);
				bs.Write<int>(tarIndex);
				m_Server->SendObjPacket(&bs);
			}
			if (m_SyncVelocity && m_isServer)
			{
				if (m_RigidBody)
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write(BaseNet_Comp::MessageTypes::Physics);
					//glm::vec3 vel = m_GameObject->GetComponent<RigidBodyComponent>()->GetVelocity();
					//printf("Set Enemey Velocity to (%f, %f, %f)\n", vel.x, vel.y, vel.z);
					bs.Write<glm::vec3>(m_RigidBody->GetVelocity());
					m_Server->SendObjPacket(&bs);
				}
			}
			//{
			//	RakNet::BitStream bs;
			//	bs.Write(GetNetworkID());
			//	bs.Write(MessageTypes::ForwardVector);
			//	bs.Write<glm::vec4>(m_GameObject->m_Transform[2]);
			//	m_Server->SendObjPacket(&bs);
			//}
			elpsTime -= m_SyncRate;
		}
	}
	//else
	//{
	//	elpsTime += deltaTime;
	//	if (this->m_SyncRate < elpsTime)
	//	{
	//		//if ((*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
	//		//{
	//		//	RakNet::BitStream bs;
	//		//	bs.Write(GetNetworkID());
	//		//	bs.Write(MessageTypes::ForwardVector);
	//		//	bs.Write<glm::vec4>(m_GameObject->m_Transform[2]);
	//		//	m_Client->SendObjPacket(&bs);
	//		//}

	//		elpsTime -= m_SyncRate;
	//	}
	//}
}

NetEnemyRangeComp::NetEnemyRangeComp()
{
	/*std::vector<Object*> tmp = m_GameObject->m_ObjManager->GetObjectsByTag(Object::Player);
	for (unsigned int i = 0; i < tmp.size(); i++)
	{
	m_playerTransforms.push_back(tmp[i]->m_Transform);
	}*/

	SetSyncRate(.1f);
	SetSyncTransform(true);
	SetSyncVelocity(true);
	AgroDistance = 15.0f;
	maxDotResult = .5f;
	atkRange = 7.0f;

	SetState(EnemyStates::Idle);

}

NetEnemyRangeComp::~NetEnemyRangeComp()
{
	//if (my_nav)
	//	delete my_nav;
}

void NetEnemyRangeComp::ExecuteState(const float & deltaTime)
{
	timeSinceAttack += deltaTime;

	DealWithColorShit(deltaTime);

	switch (m_State)
	{
	case NetEnemyRangeComp::EnemyStates::Idle:
	{
		//Check Player Vision
		if (CheckPlayerVision(deltaTime))
		{
			SetState(EnemyStates::Chase);
		}
	}break;

	case NetEnemyRangeComp::EnemyStates::Chase:
	{
		//Path and chase player
		if (CheckPlayerVision(deltaTime))
		{
			EnemyPath(deltaTime);
			Object * temp = nullptr;

			//int tar = m_CurrPath.size() - 2;

			//if (tar >= 1)
			//temp = m_CurrPath[tar];
			//else
			//temp = target;
			if (m_CurrPathIndx > (unsigned int)m_CurrPath.size())
			{
				temp = target;
			}
			else
			{
				temp = m_CurrPath[m_CurrPathIndx];
			}
			if (length2(temp->m_Transform[3] - m_GameObject->m_Transform[3]) < .01f)
			{
				m_CurrPathIndx--;
				//temp = m_CurrPath[m_CurrPathIndx];
			}
			if (temp != nullptr)
			{
				glm::vec4 nDir = (temp->m_Transform[3] - m_GameObject->m_Transform[3]);
				//printf("nDir: %f %f %f\n", nDir.x, nDir.y, nDir.z);
				nDir.y = 0;
				nDir.w = 0;
				float length2 = dot(nDir, nDir);
				//if (length2 >= 0.0001f)
				//{
				if (length2 == 0)
				{
					break;
				}

				//if (length2 < (atkRange*atkRange) + FLT_EPSILON  && temp == target)
				if (glm::length2(m_GameObject->m_Transform[3] - target->m_Transform[3]) < (atkRange*atkRange))
				{
					if (timeSinceAttack > 1.0f)
					{
						nDir = target->m_Transform[3] - m_GameObject->m_Transform[3];
						nDir = glm::normalize(nDir);
						//printf("nDir: %f\n", nDir.y);
						//Rotate enemy to face the player.
						glm::vec3 x = cross(glm::vec3(0, 1, 0), glm::vec3(nDir));
						//glm::vec3 y = cross(glm::vec3(nDir), x);
						m_GameObject->m_Transform[0].x = x.x;
						m_GameObject->m_Transform[0].y = x.y;
						m_GameObject->m_Transform[0].z = x.z;

						//m_GameObject->m_Transform[1].x = y.x;
						//m_GameObject->m_Transform[1].y = y.y;
						//m_GameObject->m_Transform[1].z = y.z;

						m_GameObject->m_Transform[2] = nDir;
						if (m_ServSendTransform)
						{
							RakNet::BitStream bs;
							bs.Write(GetNetworkID());
							bs.Write(BaseNet_Comp::MessageTypes::Transform);
							bs.Write<glm::mat4x4>(m_GameObject->m_Transform);
							m_Server->SendObjPacket(&bs);
						}
						glm::vec3 zahro = glm::vec3(0);
						m_RigidBody->SetVelocity(zahro);
						SetState(EnemyStates::Attack);
						break;
					}
				}
				else if (length2 > (AgroDistance*AgroDistance))
				{
					SetState(EnemyStates::Idle);
					break;
				}


				nDir = glm::normalize(nDir);
				//printf("nDir: %f\n", nDir.y);
				//Rotate enemy to face the player.
				glm::vec3 x = cross(glm::vec3(0, 1, 0), glm::vec3(nDir));
				//glm::vec3 y = cross(glm::vec3(nDir), x);
				m_GameObject->m_Transform[0].x = x.x;
				m_GameObject->m_Transform[0].y = x.y;
				m_GameObject->m_Transform[0].z = x.z;

				//m_GameObject->m_Transform[1].x = y.x;
				//m_GameObject->m_Transform[1].y = y.y;
				//m_GameObject->m_Transform[1].z = y.z;

				m_GameObject->m_Transform[2] = nDir;

				// TODO: Change to rigid body.
				nDir *= m_moveSpeed; //HACK : convert this to use stat script.
				nDir.y = m_RigidBody->GetVelocity().y;
				//m_GameObject->m_Transform[3] += nDir;
				//nDir = m_GameObject->m_Transform * nDir;
				m_RigidBody->SetVelocity(nDir);
			}
		}
	}break;
	case NetEnemyRangeComp::EnemyStates::Search:
	{
		//Lost sight of player, look for it.
		//TODO: AI SEARCH Later.
		//m_GameObject->m_Transform = glm::rotate(m_GameObject->m_Transform, 1.0f * deltaTime, glm::vec3(0, 1, 0));
	}break;
	case NetEnemyRangeComp::EnemyStates::Attack:
	{
		//Attack player
		Attack(deltaTime);
	}break;
	case NetEnemyRangeComp::EnemyStates::Stunned:
	{
		//CC'd Idk if I want this state.
		//glm::vec3 zahro = glm::vec3(0);
		//m_RigidBody->SetVelocity(zahro);
		Stun(deltaTime);
	}break;
	case NetEnemyRangeComp::EnemyStates::Dead:
	{

	}break;
	default:
		break;
	}
}

void NetEnemyRangeComp::Stun(float deltaTime)
{
	timeSinceStunned += deltaTime;

	if (timeSinceStunned > stunDuration)
	{
		SetState(EnemyStates::Idle);
	}
}

bool NetEnemyRangeComp::CheckPlayerVision(const float & deltaTime)
{
	//This code is like a view cone.
	//Raycast players, which can I see?
	size_t closestIndex = 0;
	bool canSee = false;
	float closestDist = 0;
	m_CheckTimer += deltaTime;
	if (m_CheckTimer >= .5f)
	{
		for (size_t i = 0; i < m_Players.size(); ++i)
		{
			glm::vec4 point = m_GameObject->m_Transform[3];
			point[1] += 1.0f;
			//If Sqrd Distance is 
			glm::vec3 direction = m_Players[i]->m_Transform[3] - point;
			//direction.y = 0;
			float sqrDist = length2(direction);
			//if (sqrDist < (AgroDistance*AgroDistance))
			{
				glm::vec3 nDir = glm::normalize(direction);
				glm::vec4 tdir = { nDir[0],nDir[1],nDir[2], 0.0f };
				//check for if the enemy is within range of view.


				//printf("Distance: %f DotResult: %f\n", sqrDist, glm::dot(nDir, glm::vec3(m_GameObject->m_Transform[2])));
				//if (glm::dot(nDir, glm::vec3(m_GameObject->m_Transform[2])) >= maxDotResult)
				//glm::vec4 hit;
				Object *temp = CollisionDetection::RaycastAgainstClosest(point, tdir, m_GameObject->m_ObjManager);
				if (temp && temp->tag == Object::Player)
				{
					//I can see you.
					if (!canSee || (closestDist > sqrDist))
					{
						closestIndex = i;
						tarIndex = closestIndex;
						closestDist = sqrDist;
						canSee = true;
					}
				}
			}
		}
		//I have a target, switch states.
		if (canSee)
		{
			target = m_Players[closestIndex];
		}
		m_CheckTimer = 0;

	}
	else
	{
		if (target)
			canSee = true;

	}
	return canSee;
}


void NetEnemyRangeComp::EnemyPath(const float & deltaTime)
{
	glm::mat4 tempTransform = m_GameObject->m_Transform;
	glm::vec3 temp1 = m_GameObject->m_Transform[3];
	if (!target) return;
	glm::vec3 temp2 = target->m_Transform[3];
	m_RepathTimer += deltaTime;
	if (m_RepathTimer > 1.5f)
	{
		m_RepathTimer = 0.0f;
		if (navSet == false)
		{
			my_nav->Enter((int)round(m_GameObject->m_Transform[3][0]), (int)round(m_GameObject->m_Transform[3][2]), (int)round(temp2[0]), (int)round(temp2[2]));
			my_nav->update(deltaTime);
			navSet = true;
		}
		else
		{
			my_nav->SEUpdate((int)round(m_GameObject->m_Transform[3][0]), (int)round(m_GameObject->m_Transform[3][2]), (int)round(temp2[0]), (int)round(temp2[2]));
			my_nav->update(deltaTime);
		}
		m_CurrPath = my_nav->getPath();
		m_CurrPathIndx = m_CurrPath.size() - 2;
	}
}

void NetEnemyRangeComp::DealWithColorShit(float deltaTime)
{
	if (timeSinceDamaged < damageColorDuration)
	{
		timeSinceDamaged += deltaTime;
		m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else
	{
		m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		switch (m_State)
		{
		case NetEnemyRangeComp::EnemyStates::Idle:
			m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			break;
		case NetEnemyRangeComp::EnemyStates::Chase:
			m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
			break;
		case NetEnemyRangeComp::EnemyStates::Search:
			m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			break;
		case NetEnemyRangeComp::EnemyStates::Attack:
			m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
			break;
		case NetEnemyRangeComp::EnemyStates::Stunned:
			m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			break;
		default:
			break;
		}
	}
}

void NetEnemyRangeComp::Attack(const float & deltaTime)
{
	tempAtkTimer += deltaTime;
	if (tempAtkTimer > 1.25f)
	{
		//printf("I AM DONE ATTACKING YOU\n");
		m_ISHOT = false;
		SetState(EnemyStates::Idle);
	}
	else
	{
		timeSinceAttack = 0;

		if (tempAtkTimer > .3f)
		{
			//if (m_upCount % 20 == 0)
			//printf("I AM ATTACKING YOU\n");
			glm::vec3 res = glm::mix(glm::vec4(0, 0, -.25f, 1), glm::vec4(0, 0, .25f, 1), tempAtkTimer - .2f);
		}
		else if (!m_ISHOT)
		{
			m_ISHOT = true;
			m_Projectiles[m_currProj]->SetEnabled(true);
			m_Projectiles[m_currProj]->GetColliders()[0]->SetEnabled(true);
			m_Projectiles[m_currProj]->m_Transform = m_GameObject->m_Transform;
			m_Projectiles[m_currProj]->m_Transform[3][1] += 1.1f;
			m_Projectiles[m_currProj]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
			m_Projectiles[m_currProj]->GetComponent<HitManager>()->ResetObjects();
			m_Projectiles[m_currProj]->SetDamage(m_GameObject->GetComponent<StatScript>()->GetAtkDamage());
			m_currProj++;
			if (m_currProj >= 3)
			{
				m_currProj = 0;
			}

			//AtkCapsule->SetEnabled(true);
			//if (m_upCount % 20 == 0)
			//m_GameObject->GetComponent<MaterialComponent>()->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

			//printf("I AM SWINGING AT YOU\n");
		}
	}
}

void NetEnemyRangeComp::Die()
{
	auto renderComp = m_GameObject->GetComponent<MeshComponent>();
	if (renderComp)
		renderComp->SetEnabled(false);
	if (m_animationComponent)
	{
		//m_animationComponent->SetEnabled(false);
		m_animationComponent->SetAnimation(DeathAnimation);
		m_animationComponent->SetAnimationState(SingleAnimating);
		m_animationComponent->SetAnimationSpeed(1.0f);
	}
	//m_GameObject->m_Parent->SetEnabled(false);

	m_GameObject->GetComponent<SphereCollider>()->SetEnabled(false);
	if (m_Server)
	{
		switch (DropType)
		{
			//Health Pickups
		case (1):
			PickupManager::GetInstance()->SpawnHealthPickup(m_GameObject->m_Transform[3]);
			break;
			//Effect Pickups
			//case (2):
			//PickupManager::GetInstance()->SpawnEffectPickup(m_GameObject->m_Transform[3], DropValue);
			//break;
			//Item Pickups
			/*case (3):
			PickupManager::GetInstance()->SpawnItemPickup(m_GameObject->m_Transform[3], DropValue);
			break;*/
		}
	}
}

void NetEnemyRangeComp::OnUpdate(float deltaTime)
{


	if (m_GameObject->m_Transform[3].y < 0)
	{
		m_GameObject->m_Transform[3].y = 0;
		//RigidBodyComponent* rig = m_GameObject->GetComponent<RigidBodyComponent>();
		glm::vec3 vel = m_RigidBody->GetVelocity();
		vel.y = 0;
		m_RigidBody->SetVelocity(vel);
	}

	if (m_isServer)
	{
		//printf("STATE : %d\n", m_State);
		//m_RigidBody = m_GameObject->GetComponent<RigidBodyComponent>();
		//m_StatScript = m_GameObject->GetComponent<StatScript>();
		if (m_upCount >= 30)
		{
			m_Players.clear();
			m_Players = m_GameObject->m_ObjManager->GetObjectsByTag(Object::Player);
			m_upCount = 0;
		}
		m_upCount++;
		if (my_nav == nullptr)
		{
			//my_nav = new Navigation::NavMesh(*m_GameObject->m_ObjManager->GetNavMesh());
			my_nav = m_GameObject->m_ObjManager->GetNavMesh();
		}
		//if (timer <= 0.0f && m_State == EnemyStates::Chase)
		//{
		//	EnemyPath(deltaTime);
		//	timer = .01f;
		//}

		//EnemyPath(deltaTime);

		ExecuteState(deltaTime);
		ServerUpdate(deltaTime);
	}
	else
	{
		//printf("CLIENT : %d\n", m_State);
		ClientUpdate(deltaTime);
	}
	PacketUpdate(deltaTime);
	//timer -= deltaTime;
}

void NetEnemyRangeComp::OnStart()
{
	DropType = rand() % 20;

	switch (DropType)
	{
	case (2):
		DropValue = rand() % (uint16_t)TimedEffectType::numTypes;
		break;/*
			  case (3):
			  DropValue = rand() % (uint16_t)ItemIDs::numIds;
			  break;*/
	}

	m_RigidBody = m_GameObject->GetComponent<RigidBodyComponent>();
	m_StatScript = m_GameObject->GetComponent<StatScript>();

	//AtkCapsule = new CapsuleCollider(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, .25f, 1), .5f, ColliderTag::HitBox, true, false);
	//AtkCapsule->AddOnCollisionEnterEvent(EnemyAttack::WeaponCollisionEnter);
	//AtkCapsule->AddOnCollisionStayEvent(EnemyAttack::WeaponCollisionStay);
	//AtkCapsule->AddOnCollisionExitEvent(EnemyAttack::WeaponCollisionExit);
	//AtkCapsule->SetEnabled(false);
	//m_GameObject->AddComponent(AtkCapsule);
	//m_Players = m_GameObject->m_ObjManager->GetObjectsByTag(Object::Player);

	DataManager* dm = DataManager::GetInstance();
	int idx = dm->LoadMesh("Enemy-Goblin/Goblin_D_Shareyko.fbx");
	Mesh* mesh = (Mesh*)dm->Get(idx);

	dm->LoadAnimation("Enemy-Goblin/idle.fbx", idx, IdleAnimation);
	dm->LoadAnimation("Enemy-Goblin/punching.fbx", idx, AttackAnimation);
	dm->LoadAnimation("Enemy-Goblin/stun.fbx", idx, StunAnimation);
	dm->LoadAnimation("Enemy-Goblin/walk.fbx", idx, WalkAnimation);
	dm->LoadAnimation("Enemy-Goblin/head_spinning_inPlace.fbx", idx, DeathAnimation);
	m_animationComponent = new AnimationComponent(mesh);

	m_animationComponent->SetAnimation("idle");
	m_animationComponent->SetAnimationState(LoopAnimating);
	m_animationComponent->SetAnimationSpeed(1.0f);
	//m_animationComponent->SetDebug(true);
	Object* ScaleObj = new Object();
	ScaleObj->AddComponent(new MeshComponent(mesh));
	ScaleObj->AddComponent(m_animationComponent);
	ScaleObj->m_Transform = glm::scale(glm::mat4(1), glm::vec3(.01f, .01f, .01f));
	ScaleObj->SetParent(m_GameObject);//ScaleObj->m_Parent = m_GameObject;
	m_GameObject->m_ObjManager->AddObject(ScaleObj);
	m_SyncVelocity = true;
	m_RigidBody->SetIsFrozen(false);
	m_RigidBody->SetIsKinematic(true);


	//m_weaponCollider = new CapsuleCollider(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), 0.5f, ColliderTag::HitBox, true, false);
	//m_weaponCollider->SetEnabled(false);
	
	idx = dm->LoadMesh("sphere.fbx");
	for (int i = 0; i < 3; i++)
	{
		Projectile * temp = new Projectile;
		temp->m_ObjManager = m_GameObject->m_ObjManager;
		temp->SetIsPlayer(false);
		temp->SetEnabled(false);
		temp->m_Static = false;
		temp->SetOwner(m_GameObject);
		temp->m_Persistent = false;
		temp->m_Creator = m_GameObject->m_Creator;
		temp->AddComponent(new HitManager);
		temp->AddComponent(new StatScript(*m_StatScript));
		auto *mc = new MaterialComponent();
		mc->SetColor(glm::vec4(0, 0, 1, 1));
		temp->AddComponent(mc);
		SphereCollider * pc = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::HitBox, true, true);

		pc->AddOnCollisionEnterEvent(EnemyAttackRange::WeaponCollisionEnter);
		pc->AddOnCollisionStayEvent(EnemyAttackRange::WeaponCollisionStay);
		pc->AddOnCollisionExitEvent(EnemyAttackRange::WeaponCollisionExit);
		pc->SetEnabled(false);

		temp->AddComponent(pc);
		m_Projectiles.push_back(temp);
		m_GameObject->m_ObjManager->AddObject(temp);
	}
	

	//m_weapon = new Object();
	//m_weapon->m_Transform = glm::mat4(1);
	//m_weapon->AddComponent(m_weaponCollider);
	//m_weapon->tag = Object::ObjectTags::World;
	//
	//
	//idx = dm->LoadMesh("sphere.fbx");
	//
	//if (idx != LOADERROR)
	//	m_weapon->AddComponent(new MeshComponent((Mesh*)dm->Get(idx)));

	//m_GameObject->m_ObjManager->AddObject(m_weapon);
	m_GameObject->AddComponent(new SoundSourceComponent());
	m_SndSrc = m_GameObject->GetComponent<SoundSourceComponent>();
}

void NetEnemyRangeComp::ServerUpdate(const float & deltaTime)
{
	//m_StatScript->TakeDamage(1, NULL);
}

void NetEnemyRangeComp::ClientUpdate(const float & deltaTime)
{

}

void NetEnemyRangeComp::HandleMessage(RakNet::BitStream * bs)
{
	//BaseNet_Comp::HandleMessage(bs);

	MessageTypes type;
	bs->Read<MessageTypes>(type);

	switch (type)
	{
	case BaseNet_Comp::MessageTypes::Transform:
	{
		glm::mat4x4 mat;
		bs->Read<glm::mat4x4>(mat);
		m_GameObject->m_Transform = mat;
	}break;
	case MessageTypes::StateChanged:
	{
		EnemyStates temp;
		bs->Read<EnemyStates>(temp);
		SetState(temp);
	}break;
	case MessageTypes::ForwardVector:
	{
		glm::vec4 forward;
		bs->Read<glm::vec4>(forward);
		const glm::vec3 up(0, 1, 0);
		glm::vec3 x = glm::cross(up, glm::vec3(forward[0], forward[1], forward[2]));

		m_GameObject->m_Transform[0].x = x.x;
		m_GameObject->m_Transform[0].y = x.y;
		m_GameObject->m_Transform[0].z = x.z;

		m_GameObject->m_Transform[1].x = up.x;
		m_GameObject->m_Transform[1].y = up.y;
		m_GameObject->m_Transform[1].z = up.z;

		m_GameObject->m_Transform[2].x = forward.x;
		m_GameObject->m_Transform[2].y = forward.y;
		m_GameObject->m_Transform[2].z = forward.z;
	}break;
	case MessageTypes::CurrentTarget:
	{
		int temp;
		bs->Read<int>(temp);
		tarIndex = temp;
		if (m_Players.size() > 0)
			target = m_Players[tarIndex];
	}break;
	case BaseNet_Comp::MessageTypes::Health:
	{
		float nhp;
		bs->Read<float>(nhp);
		m_StatScript->SetCurrentHealth(nhp);
	}break;
	case BaseNet_Comp::MessageTypes::Physics:
	{
		glm::vec3 temp;
		bs->Read<glm::vec3>(temp);
		m_RigidBody->SetVelocity(temp);
	}break;
	default:
		break;
	}
}

NetEnemyRangeComp::EnemyStates NetEnemyRangeComp::GetState()
{
	return m_State;
}

void NetEnemyRangeComp::SetState(EnemyStates state)
{
	if (state != m_State)
	{
		ExitState(state);
		m_State = state;

		if (m_isServer)
		{
			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(MessageTypes::StateChanged);
			bs.Write<EnemyStates>(m_State);
			m_Server->SendObjPacket(&bs, RELIABLE_ORDERED);
		}

		switch (state)
		{
		case EnemyStates::Attack:
		{
			if (m_animationComponent)
			{
				m_animationComponent->SetAnimation(AttackAnimation);
				m_animationComponent->SetAnimationState(SingleAnimating);
				m_animationComponent->SetAnimationSpeed(1.0f);
			}

			if (!IsServer())
			{
				m_Projectiles[m_currProj]->SetEnabled(true);
				m_Projectiles[m_currProj]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
				m_Projectiles[m_currProj]->m_Transform = m_GameObject->m_Transform;
				m_Projectiles[m_currProj]->m_Transform[3][1] += 1.1f;
				m_Projectiles[m_currProj]->m_Transform = m_Projectiles[m_currProj]->m_Transform * glm::scale(glm::mat4(1), glm::vec3(.5, .5, .5));
				m_currProj++;
				if (m_currProj >= 3)
				{
					m_currProj = 0;
				}
			}

			tempAtkTimer = 0;
			EnemyAttackRange::atkcnt++;

			break;
		}

		case EnemyStates::Chase:
		{
			if (m_animationComponent)
			{
				m_animationComponent->SetAnimation(WalkAnimation);
				m_animationComponent->SetAnimationState(LoopAnimating);
				m_animationComponent->SetAnimationSpeed(1.0f);
			}

			break;
		}

		case EnemyStates::Idle:
		{
			if (m_animationComponent)
			{
				m_animationComponent->SetAnimation(IdleAnimation);
				m_animationComponent->SetAnimationState(LoopAnimating);
				m_animationComponent->SetAnimationSpeed(1.0f);
			}

			break;
		}

		case EnemyStates::Search:
		{
			if (m_animationComponent)
			{
				m_animationComponent->SetAnimation(WalkAnimation);
				m_animationComponent->SetAnimationState(LoopAnimating);
				m_animationComponent->SetAnimationSpeed(1.0f);
			}


			break;
		}

		case EnemyStates::Stunned:
		{
			if (m_animationComponent)
			{
				m_animationComponent->SetAnimation(StunAnimation);
				m_animationComponent->SetAnimationState(LoopAnimating);
				m_animationComponent->SetAnimationSpeed(1.0f);
			}

			timeSinceStunned = 0;

			break;
		}
		case EnemyStates::Dead:
		{
			//auto renderComp = m_GameObject->GetComponent<MeshComponent>();
			//if (renderComp)
			//	renderComp->SetEnabled(false);
			//if (m_animationComponent)
			//	m_animationComponent->SetEnabled(false);
			////m_GameObject->m_Parent->SetEnabled(false);
			//
			//m_animationComponent->SetAnimation(DeathAnimation);
			//m_animationComponent->SetAnimationState(SingleAnimating);
			//m_animationComponent->SetAnimationSpeed(1.0f);
			//
			//if (m_Server)
			//{
			//	switch (DropType)
			//	{
			//		//Health Pickups
			//	case (1):
			//		PickupManager::GetInstance()->SpawnHealthPickup(m_GameObject->m_Transform[3]);
			//		break;
			//		//Effect Pickups
			//		//case (2):
			//		//PickupManager::GetInstance()->SpawnEffectPickup(m_GameObject->m_Transform[3], DropValue);
			//		//break;
			//		//Item Pickups
			//		/*case (3):
			//		PickupManager::GetInstance()->SpawnItemPickup(m_GameObject->m_Transform[3], DropValue);
			//		break;*/
			//	}
			//}
			Die();
			break;
		}
		}
	}
}

void NetEnemyRangeComp::ExitState(EnemyStates newState)
{
	switch (m_State)
	{
	case EnemyStates::Idle:
	{
		break;
	}

	case EnemyStates::Chase:
	{
		break;
	}

	case EnemyStates::Search:
	{
		break;
	}

	case EnemyStates::Attack:
	{
		//AtkCapsule->SetEnabled(false);
		break;
	}

	case EnemyStates::Stunned:
	{
		break;
	}

	default:
		break;
	}
}