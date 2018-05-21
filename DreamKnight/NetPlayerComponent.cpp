#include "NetPlayerComponent.h"
#include "NetClient.h"
#include "NetServer.h"
#include "InputManager.h"
#include "Collider.h"
#include "CapsuleCollider.h"
#include "StatScript.h"
#include "InventorySubComponent.h"
#include "AnimationComponent.h"
#include "MaterialComponent.h"
#include "ScreamingComponent.h"
#include "NetEnemyComp.h"
#include "NetEnemyRangeComp.h"
#include "HitManagerComponent.h"
#include "TimedEffectsSubComponent.h"
#include "glm.h"
#include "ThirdPersonCameraComponent.h"
#include "DKEngine.h"
#include "GuiHealthbar.h"
#include "NetChestComponent.h"

#define MixamoWarrior 0
#define PrintDebugInfo 0

//Animation Names for Various States
#define Idle_Animation					"idle"
#define PrimaryAttack_Combo1_Animation	"slash1"
#define PrimaryAttack_Combo2_Animation	"slash2"
#define SecondaryAttack_Animation		"secondary"
#define TertiaryAttack_Animation		"tertiary"
#define SuperAbility_Animation			"super"
#define Defensive_Animation				"battlecry"
#define Utility_Animation				"roll"
#define Walk_Animation					"run"
#define StrafeLeft_Animtion				"strafe_left"
#define StrafeRight_Animtion			"strafe_right"
#define Jump_Animation					"jump"
#define HitStun_Animation				"stun"
#define Death_Animation					"death"

#define PrimaryAttack_Combo1_Sound	"sounds/DK_SwordSlash_02.ogg"
#define PrimaryAttack_Combo2_Sound	"sounds/DK_SwordSlash_02.ogg"
#define SecondaryAttack_Sound		"sounds/sword.ogg"
#define TertiaryAttack_Sound		"sounds/sword.ogg"
#define SuperAbility_Sound			""
#define DefensiveAbility_Sound		""
#define UtilityAbility_Sound		""
#define Walk_Sound					"sounds/DK_Footstep_Left.ogg"
#define StrafeLeft_Sound			""
#define StrafeRight_Sound			""
#define Jump_Sound					""
#define Hit_Sound					"sounds/DK_PlayerDamage.ogg"
#define Death_Sound					"sounds/DK_PlayerDeath.ogg"

//Maximum values and modifier values
#define TerminalVelocity -25.0f
#define RunMoveSpeed 2.3f
#define AttackMoveSpeed 0.4f

//Debug Include
#include "ItemRef.h"

#if PrintDebugInfo

#include <sstream>
namespace printFlags
{
	bool WeaponEnter = false;
	bool WeaponStay = false;
	bool WeaponExit = false;
}

#endif

namespace PlayerCollisions
{
#pragma optimize("",off)
	void WeaponCollisionEnter(CollisionEvent e)
	{
		//Utils::PrintWarning("I ENETER\n");

		if (e.m_CollidingWith->GetColliderTag() != ColliderTag::HurtBox)
			return;

#if PrintDebugInfo
		if (WeaponEnter && e.m_CollidingWith->m_GameObject->tag != Object::ObjectTags::World)
		{
			std::stringstream outString;
			outString << (e.m_ThisObject->m_GameObject->GetComponent<BaseNet_Comp>()->IsServer() ? "SERVER" : "CLIENT") << ": ENTER" << std::endl;
			outString << "\tFunctionCall: WeaponCollisionEnter" << ":\tPlayer 1" << std::endl;
			outString << "\tColliding With: " << e.m_CollidingWith->m_GameObject->tag << std::endl;
			printf_s(outString.str().data());
			outString.clear();
		}
#endif
		//printf("I ENETER\n");
		Utils::PrintWarning("I enter, sword\n");
		if (e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Enemy)
		{
			if (e.m_ThisObject->m_GameObject->GetComponent<HitManager>()->RegisterHit(e.m_CollidingWith->m_GameObject))
			{
				printf("I AM IN\n");
				NetEnemyComp* enemy=NULL;
				NetEnemyRangeComp* enemyR=NULL;
				NetPlayerComponent* player = e.m_ThisObject->m_GameObject->GetParent()->GetComponent<NetPlayerComponent>();
				StatScript* playerStats = player->m_GameObject->GetComponent<StatScript>();
				StatScript* enemyStats=NULL;// = enemy->m_GameObject->GetComponent<StatScript>();
				if (e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>() != NULL)
				{
					enemy = e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>();
					enemy->timeSinceDamaged = 0.0f;
					enemyStats = enemy->m_GameObject->GetComponent<StatScript>();
					//e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>()->m_State = NetEnemyComp::EnemyStates::Stunned;
					if (enemy->GetState() == NetEnemyComp::EnemyStates::Dead)
						return;
				}
				if (e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyRangeComp>() != NULL)
				{
					enemyR = e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyRangeComp>();
					enemyR->timeSinceDamaged = 0.0f;
					enemyStats = enemyR->m_GameObject->GetComponent<StatScript>();
					//e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>()->m_State = NetEnemyComp::EnemyStates::Stunned;
					if (enemyR->GetState() == NetEnemyRangeComp::EnemyStates::Dead)
						return;
				}
				float damage = playerStats->GetAtkDamage() * player->GetDamageModifier();

				if (player && player->m_Inventory)
				{
					if (enemy != NULL)
						player->m_Inventory->OnAttackHit(e.m_ThisObject->m_GameObject, enemy->m_GameObject, damage);
					if (enemyR != NULL)
						player->m_Inventory->OnAttackHit(e.m_ThisObject->m_GameObject, enemyR->m_GameObject, damage);
				}
				if (player && player->m_TemporaryEffects)
				{
					if (enemy != NULL)
						player->m_TemporaryEffects->OnAttackHit(e.m_ThisObject->m_GameObject, enemy->m_GameObject, damage);
					if (enemyR != NULL)
						player->m_TemporaryEffects->OnAttackHit(e.m_ThisObject->m_GameObject, enemyR->m_GameObject, damage);
				}


				if (player->GetState() == PlayerStates::SecondaryAttack)
				{
					if (enemy)
					{
						glm::vec3 knockbackVector = glm::normalize(enemy->m_GameObject->GetWorldMatrix()[3] - player->m_GameObject->GetWorldMatrix()[3]);
						float knockbackStrength = 3;
						enemy->m_GameObject->GetComponent<RigidBodyComponent>()->SetVelocity(knockbackVector * knockbackStrength);
					}
					else
					{
						
						glm::vec3 knockbackVector = glm::normalize(enemyR->m_GameObject->GetWorldMatrix()[3] - player->m_GameObject->GetWorldMatrix()[3]);
						float knockbackStrength = 3;
						enemyR->m_GameObject->GetComponent<RigidBodyComponent>()->SetVelocity(knockbackVector * knockbackStrength);
					}
				}

				if (enemyStats)
				{
					enemyStats->TakeDamage(damage, player->m_GameObject);
					enemyStats->AddStunValue(player->GetStunValue());

					printf("\tRemaining HP: %f", enemyStats->GetCurrentHealth());
					//printf("IDUNHITTEDIT\n");
				}
				else //if (player->IsServer())
				{
					printf("I MISSED SOMEHOW\n");
				}
			}
		}
		else if (e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Chest)
		{
			NetChestComp* NCC = e.m_CollidingWith->m_GameObject->GetComponent<NetChestComp>();
			if (NCC)
			{
				NCC->DropItem(e.m_ThisObject->m_GameObject->m_Transform[3]);
			}
		}

	}

	void ProjectileCollisionEnter(CollisionEvent e)
	{
		//Utils::PrintWarning("I ENETER\n");

		if (e.m_CollidingWith->m_GameObject->tag == Object::Wall)
		{
			e.m_ThisObject->m_GameObject->SetEnabled(false);
			return;
		}

		if (e.m_CollidingWith->GetColliderTag() != ColliderTag::HurtBox)
			return;
		

#if PrintDebugInfo
		if (WeaponEnter && e.m_CollidingWith->m_GameObject->tag != Object::ObjectTags::World)
		{
			std::stringstream outString;
			outString << (e.m_ThisObject->m_GameObject->GetComponent<BaseNet_Comp>()->IsServer() ? "SERVER" : "CLIENT") << ": ENTER" << std::endl;
			outString << "\tFunctionCall: WeaponCollisionEnter" << ":\tPlayer 1" << std::endl;
			outString << "\tColliding With: " << e.m_CollidingWith->m_GameObject->tag << std::endl;
			printf_s(outString.str().data());
			outString.clear();
		}
#endif
		//printf("I ENETER\n");
		//Utils::PrintWarning("I enter, projectile\n");
		if (e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Enemy)
		{
			if (e.m_ThisObject->m_GameObject->GetComponent<HitManager>()->RegisterHit(e.m_CollidingWith->m_GameObject))
			{
				printf("I AM IN\n");
				//e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>()->m_State = NetEnemyComp::EnemyStates::Stunned;

				NetEnemyComp* enemy = e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyComp>();
				NetEnemyRangeComp* enemyR = e.m_CollidingWith->m_GameObject->GetComponent<NetEnemyRangeComp>();
				NetPlayerComponent* player = ((Projectile*)e.m_ThisObject->m_GameObject)->GetOwner()->GetComponent<NetPlayerComponent>();
				//StatScript* playerStats = player->m_GameObject->GetComponent<StatScript>();
				StatScript* enemyStats;// = enemy->m_GameObject->GetComponent<StatScript>();

				if (enemy != NULL)
				{
					enemy->timeSinceDamaged = 0.0f;
					enemyStats = enemy->m_GameObject->GetComponent<StatScript>();
				}
				if (enemyR != NULL)
				{
					enemyR->timeSinceDamaged = 0.0f;
					enemyStats = enemyR->m_GameObject->GetComponent<StatScript>();
				}

				float damage = ((Projectile*)e.m_ThisObject->m_GameObject)->GetDamage();//playerStats->GetAtkDamage() * player->GetDamageModifier();

				if (player && player->m_Inventory)
				{
					if (enemy != NULL)
						player->m_Inventory->OnAttackHit(e.m_ThisObject->m_GameObject, enemy->m_GameObject, damage);
					if (enemyR != NULL)
						player->m_Inventory->OnAttackHit(e.m_ThisObject->m_GameObject, enemyR->m_GameObject, damage);
				}
				if (player && player->m_TemporaryEffects)
				{
					if (enemy != NULL)
						player->m_TemporaryEffects->OnAttackHit(e.m_ThisObject->m_GameObject, enemy->m_GameObject, damage);
					if (enemyR != NULL)
						player->m_TemporaryEffects->OnAttackHit(e.m_ThisObject->m_GameObject, enemyR->m_GameObject, damage);
				}

				if (enemyStats && player)// && player->IsServer())
				{
					enemyStats->TakeDamage(damage, player->m_GameObject);
					enemyStats->AddStunValue(player->GetStunValue());

					printf("\tRemaining HP: %f", enemyStats->GetCurrentHealth());
					//printf("IDUNHITTEDIT\n");
				}
				else //if (player->IsServer())
				{
					printf("I MISSED SOMEHOW\n");
				}
			}
		}
		else if (e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Chest)
		{
			NetChestComp* NCC = e.m_CollidingWith->m_GameObject->GetComponent<NetChestComp>();
			if (NCC)
			{
				NCC->DropItem(e.m_ThisObject->m_GameObject->m_Transform[3]);
			}
		}


	}

	void WeaponCollisionStay(CollisionEvent e)
	{
		//Utils::PrintWarning("I stay\n");
		if (e.m_CollidingWith->GetColliderTag() != ColliderTag::HurtBox)
			return;

#if PrintDebugInfo
		if (WeaponStay && e.m_CollidingWith->m_GameObject->tag != Object::ObjectTags::World)
		{
			std::stringstream outString;
			outString << (e.m_ThisObject->m_GameObject->GetComponent<BaseNet_Comp>()->IsServer() ? "SERVER" : "CLIENT") << ": STAY" << std::endl;
			outString << "\tFunction Call: WeaponCollisionStay" << ":\tPlayer 1" << std::endl;
			outString << "\tColliding With: " << e.m_CollidingWith->m_GameObject->tag << std::endl;
			//printf_s(outString.str().data());
			outString.clear();
		}
#endif
	}

	void WeaponCollisionExit(CollisionEvent e)
	{
		if (e.m_CollidingWith->GetColliderTag() != ColliderTag::HurtBox)
			return;
		//Utils::PrintWarning("I exit\n");

#if PrintDebugInfo
		if (WeaponExit && e.m_CollidingWith->m_GameObject->tag != Object::ObjectTags::World)
		{
			std::stringstream outString;
			outString << (e.m_ThisObject->m_GameObject->GetComponent<BaseNet_Comp>()->IsServer() ? "SERVER" : "CLIENT") << ": EXIT" << std::endl;
			outString << "\tFunctionCall: WeaponCollisionExit" << ":\tPlayer 1" << std::endl;
			outString << "\tColliding With: " << e.m_CollidingWith->m_GameObject->tag << std::endl;
			printf_s(outString.str().data());
			outString.clear();
		}
#endif
	}
}

NetPlayerComponent::NetPlayerComponent()
{
	m_Animator = nullptr;
	m_WeaponCollider = nullptr;
	m_Weapon = nullptr;
	m_Input = nullptr;
	m_Inventory = nullptr;
	m_TemporaryEffects = nullptr;
	m_PrimaryAttack_Combo1_Time = 0.0f;
	m_PrimaryAttack_Combo2_Time = 0.0f;
	SetState(PlayerStates::Idle);
	m_SyncForwardVec = true;
}

float NetPlayerComponent::GetDamageModifier()
{
	switch (m_CurrState)
	{
		case PlayerStates::Utility:
			return 0.0f;
		case PlayerStates::Defensive:
			return 0.0f;
		case PlayerStates::PrimaryAttack_Combo1:
			return m_PrimaryAttack_Combo1_DamageModifier;
		case PlayerStates::PrimaryAttack_Combo2:
			return m_PrimaryAttack_Combo2_DamageModifier;
		case PlayerStates::SecondaryAttack:
			return m_SecondaryAttack_DamageModifier;
		case PlayerStates::TertiaryAttack:
			return m_TertiaryAttack_DamageModifier;
		case PlayerStates::SuperAbility:
			return m_SuperAbility_DamageModifier;
		default:
			return 0.0f;
	}
}

float NetPlayerComponent::GetDamage()
{
	return m_Stats->GetAtkDamage();
}

float NetPlayerComponent::GetStunValue()
{
	switch (m_CurrState)
	{
		case PlayerStates::PrimaryAttack_Combo1:
			return m_PrimaryAttack_Combo1_HitStunValue;
		case PlayerStates::PrimaryAttack_Combo2:
			return m_PrimaryAttack_Combo2_HitStunValue;
		case PlayerStates::SecondaryAttack:
			return m_SecondaryAttack_HitStunValue;
		case PlayerStates::TertiaryAttack:
			return m_TertiaryAttack_HitStunValue;
		case PlayerStates::SuperAbility:
			return m_SuperAbility_HitStunValue;
		default:
			return 0.0f;
	}
}

void NetPlayerComponent::FireProjectile(float Damage)
{
	if (!proj[projItr]->isEnabled())
	{
		proj[projItr]->SetEnabled(true);
		proj[projItr]->GetComponent<SphereCollider>()->SetEnabled(true);
		proj[projItr]->m_Transform = m_GameObject->m_Transform;
		proj[projItr]->m_Transform[3][1] += 0.5f;
		proj[projItr]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
		proj[projItr]->GetComponent<HitManager>()->ResetObjects();

		Utils::PrintWarning("Projectile info: #: %d, Collider Enabled: %s\n",
			projItr,
			(proj[projItr]->GetComponent<SphereCollider>()->IsEnabled()) ? "True" : "False");

		if (Damage > 0.0f)
		{
			proj[projItr]->SetDamage(Damage);
		}
		else
		{
			proj[projItr]->SetDamage(m_Stats->GetAtkDamage() * GetDamageModifier());
		}

		projItr++;
	}
	if (projItr >= 10)
		projItr = 0;
}

NetPlayerComponent::~NetPlayerComponent()
{
	if (m_Inventory)
		delete m_Inventory;
	if (m_TemporaryEffects)
		delete m_TemporaryEffects;
}

void NetPlayerComponent::HandleCoolDowns(float deltaTime)
{
	m_PrimaryAttack_Combo1_CurrentCooldownTime += deltaTime;
	m_PrimaryAttack_Combo2_CurrentCooldownTime += deltaTime;
	m_SecondaryAttack_CurrentCooldownTime += deltaTime;
	m_TertiaryAttack_CurrentCooldownTime += deltaTime;
	m_SuperAbility_CurrentCooldownTime += deltaTime;

	if (m_PrimaryAttack_Combo1_CurrentCooldownTime > m_PrimaryAttack_Combo1_CooldownDuration)
		m_PrimaryAttack_Combo1_CoolingDown = false;

	if (m_PrimaryAttack_Combo2_CurrentCooldownTime > m_PrimaryAttack_Combo2_CooldownDuration)
		m_PrimaryAttack_Combo2_CoolingDown = false;

	if (m_SecondaryAttack_CurrentCooldownTime > m_SecondaryAttack_CooldownDuration)
		m_SecondaryAttack_CoolingDown = false;

	if (m_TertiaryAttack_CurrentCooldownTime > m_TertiaryAttack_CooldownDuration)
		m_TertiaryAttack_CoolingDown = false;

	if (m_SuperAbility_CurrentCooldownTime > m_SuperAbility_CooldownDuration)
		m_SuperAbility_CoolingDown = false;
}

#pragma region StateMachine
void NetPlayerComponent::HandleState(float deltaTime)
{
	HandleCoolDowns(deltaTime);

	switch (m_CurrState)
	{
		case PlayerStates::Idle:
		{
			if (HandleJump())
			{
				break;
			}

			if (HandleUtilityAbility())
			{
				break;
			}

			if (HandleMovement())
			{
				// DO Stuff
			}

			if (HandleDefensiveAbility())
			{

			}

			if (HandlePrimaryAttack())
			{
				break;
			}

			if (HandleSecondaryAttack())
			{
				break;
			}

			if (HandleTertiaryAttack())
			{
				break;
			}

			if (HandleSuperAbility())
			{
				break;
			}

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::Move:
		{
			if (HandleJump())
			{
				break;
			}

			if (HandleUtilityAbility())
			{
				break;
			}

			if (HandleMovement())
			{
				// Do stuff
			}

			if (HandleDefensiveAbility())
			{

			}

			if (HandlePrimaryAttack())
			{
				break;
			}

			if (HandleSecondaryAttack())
			{
				break;
			}

			if (HandleTertiaryAttack())
			{
				break;
			}

			if (HandleSuperAbility())
			{
				break;
			}

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::Jump:
		{
			if (HandleDefensiveAbility())
			{

			}

			break;
		}

		case PlayerStates::Utility:
		{
			if (HandleDefensiveAbility())
			{

			}

			glm::vec4 desiredDir = glm::vec4(0);

			desiredDir.x = (float)(m_Input->IsKeyPressed(InputManager::KEYMAP::RIGHT) - m_Input->IsKeyPressed(InputManager::KEYMAP::LEFT));
			desiredDir.z = (float)(m_Input->IsKeyPressed(InputManager::KEYMAP::FORWARD) - m_Input->IsKeyPressed(InputManager::KEYMAP::BACKWARD));

			if (desiredDir.x == 0 && desiredDir.z == 0)
				desiredDir.z = 1.0f;

			glm::vec4 previousY = glm::vec4(0);
			previousY.y = m_RigidBody->GetVelocity().y;
			m_RigidBody->SetVelocity((m_GameObject->m_Transform *  normalize(desiredDir) * m_UtilitySpeed) + previousY);

			if (m_UtilityTime < m_UtilityDuration)
			{
				m_UtilityTime += deltaTime;
			}
			else
			{
				m_RigidBody->SetConstantForce(glm::vec3(0, 0, 0));
				m_RigidBody->CancelAcceleration(glm::vec3(0, 0, 1));
				m_RigidBody->CancelVelocity(glm::vec3(0, 0, 1));
				SetState(PlayerStates::Idle);
			}

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::Defensive:
		{
			if (m_DefensiveTime < m_DefensiveDuration)
			{
				m_DefensiveTime += deltaTime;
			}
			else
			{
				SetState(PlayerStates::Idle);
			}

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::PrimaryAttack_Combo1:
		{
			if (m_Input->IsKeyDown(InputManager::KEYMAP::PRIMARY_ATTACK))
			{
				m_comboBreaker = false;
			}

			if (m_PrimaryAttack_Combo1_Time < m_PrimaryAttack_Combo1_Duration)
			{
				m_PrimaryAttack_Combo1_Time += deltaTime * m_Stats->GetAtkSpeed();
			}
			else
			{
				if (m_comboBreaker)
				{
					SetState(PlayerStates::Idle);
					m_WeaponCollider->SetEnabled(false);
				}
				else
				{
					m_Inventory->OnAttack(m_GameObject);	
					SetState(PlayerStates::PrimaryAttack_Combo2);
					m_PrimaryAttack_Combo1_Time = 0;
					m_comboBreaker = true;
				}
			}

			break;
		}
		case PlayerStates::PrimaryAttack_Combo2:
		{
			if (m_Input->IsKeyDown(InputManager::KEYMAP::PRIMARY_ATTACK))
			{
				m_comboBreaker = false;
			}

			if (m_PrimaryAttack_Combo2_Time < m_PrimaryAttack_Combo2_Duration)
			{
				m_PrimaryAttack_Combo2_Time += deltaTime * m_Stats->GetAtkSpeed();
			}

			else
			{
				if (m_comboBreaker)
				{
					SetState(PlayerStates::Idle);
					m_WeaponCollider->SetEnabled(false);
				}
				else
				{
					m_Inventory->OnAttack(m_GameObject);					
					SetState(PlayerStates::PrimaryAttack_Combo1);
					m_PrimaryAttack_Combo2_Time = 0;
					m_comboBreaker = true;
				}
			}

			break;
		}

		case PlayerStates::SecondaryAttack:
		{
			if (m_SecondaryAttackTime < m_SecondaryAttackDuration)
			{
				m_SecondaryAttackTime += deltaTime * m_Stats->GetAtkSpeed();
			}
			else
			{
				SetState(PlayerStates::Idle);
				m_WeaponCollider->SetEnabled(false);
			}

			break;
		}

		case PlayerStates::TertiaryAttack:
		{
			if (m_TertiaryAttackTime < m_TertiaryAttackDuration)
			{
				m_TertiaryAttackTime += deltaTime * m_Stats->GetAtkSpeed();
			}
			else
			{
				SetState(PlayerStates::Idle);
				m_WeaponCollider->SetEnabled(false);
			}

			break;
		}

		case PlayerStates::SuperAbility:
		{
			if (m_SuperAbilityTime < m_SuperAbilityDuration)
			{
				m_SuperAbilityTime += deltaTime * m_Stats->GetAtkSpeed();
			}
			else
			{
				SetState(PlayerStates::Idle);
				m_WeaponCollider->SetEnabled(false);
			}

			break;
		}

		case PlayerStates::Dead:
		{
			break;
		}

		case PlayerStates::Invalid:
		{
			break;
		}
	}
}

bool NetPlayerComponent::HandleMovement(bool allowStateChange)
{
	bool move = false;

	glm::vec4 desiredDir = glm::vec4(0);

	desiredDir.z = (float)(m_Input->IsKeyPressed(InputManager::KEYMAP::FORWARD) - m_Input->IsKeyPressed(InputManager::KEYMAP::BACKWARD));
	desiredDir.x = (float)(m_Input->IsKeyPressed(InputManager::KEYMAP::RIGHT) - m_Input->IsKeyPressed(InputManager::KEYMAP::LEFT));

	if (desiredDir.x != 0.0f || desiredDir.z != 0.0f)
	{
		if (allowStateChange)
			SetState(PlayerStates::Move);

		desiredDir = normalize(desiredDir) * m_Stats->GetMoveSpeed() * m_Stats->GetMoveSpeedModifier();
		move = true;
	}
	else
		if (allowStateChange)
			SetState(PlayerStates::Idle);

	desiredDir = m_GameObject->m_Transform * desiredDir;
	desiredDir.y = m_RigidBody->GetVelocity().y;
	m_RigidBody->SetVelocity(desiredDir);

	return move;
}

bool NetPlayerComponent::HandleJump()
{
	bool jump = false;

	if (m_Input->IsKeyDown(InputManager::KEYMAP::JUMP))
	{
		SetState(PlayerStates::Jump);
		m_RigidBody->AddVelocity(glm::vec3(0.0f, 5.0f, 0.0f));
		jump = true;
	}

	return jump;
}

bool NetPlayerComponent::HandleUtilityAbility()
{
	bool charge = false;

	if (m_Input->IsKeyDown(InputManager::KEYMAP::MOBILE_ABILITY))
	{
		SetState(PlayerStates::Utility);
		charge = true;
	}

	return charge;
}

bool NetPlayerComponent::HandleDefensiveAbility()
{
	return false;
	bool defensive = false;

	if (m_Input->IsKeyDown(InputManager::KEYMAP::DEFENSE_ABILITY))
	{
		SetState(PlayerStates::Defensive);
		defensive = true;
	}

	return defensive;
}

bool NetPlayerComponent::HandlePrimaryAttack()
{
	if (m_PrimaryAttack_Combo1_CoolingDown)
		return false;

	if (m_CurrState != PlayerStates::PrimaryAttack_Combo1 && m_Input->IsKeyDown(InputManager::KEYMAP::PRIMARY_ATTACK))
	{
		SetState(PlayerStates::PrimaryAttack_Combo1);
		m_comboBreaker = true;
		m_PrimaryAttack_Combo1_CoolingDown = true;
		m_PrimaryAttack_Combo1_CurrentCooldownTime = 0.0f;

		/*if (m_Inventory)
			m_Inventory->OnAttack(m_GameObject);*/
		/*if (m_TemporaryEffects)
			m_TemporaryEffects->OnAttack(m_GameObject);*/

		return true;
	}

	return false;
}

bool NetPlayerComponent::HandleSecondaryAttack()
{
	if (m_SecondaryAttack_CoolingDown)
		return false;

	if (m_CurrState != PlayerStates::SecondaryAttack && m_Input->IsKeyDown(InputManager::KEYMAP::SECONDARY_ATTACK))
	{
		SetState(PlayerStates::SecondaryAttack);
		m_SecondaryAttack_CoolingDown = true;
		m_SecondaryAttack_CurrentCooldownTime = 0.0f;

		if (m_Inventory)
			m_Inventory->OnAttack(m_GameObject);
		if (m_TemporaryEffects)
			m_TemporaryEffects->OnAttack(m_GameObject);

		return true;
	}

	return false;
}

bool NetPlayerComponent::HandleTertiaryAttack()
{
	if (m_TertiaryAttack_CoolingDown)
		return false;

	if (m_CurrState != PlayerStates::TertiaryAttack && m_Input->IsKeyDown(InputManager::KEYMAP::TERTIARY_ATTACK))
	{
		SetState(PlayerStates::TertiaryAttack);
		m_TertiaryAttack_CoolingDown = true;
		m_TertiaryAttack_CurrentCooldownTime = 0.0f;

		if (m_Inventory)
			m_Inventory->OnAttack(m_GameObject);
		if (m_TemporaryEffects)
			m_TemporaryEffects->OnAttack(m_GameObject);

		return true;
	}

	return false;
}

bool NetPlayerComponent::HandleSuperAbility()
{
	if (m_SuperAbility_CoolingDown)
		return false;

	if (m_CurrState != PlayerStates::SuperAbility && m_Input->IsKeyDown(InputManager::KEYMAP::SUPER_ABILITY))
	{
		SetState(PlayerStates::SuperAbility);
		m_SuperAbility_CoolingDown = true;
		m_SuperAbility_CurrentCooldownTime = 0.0f;

		//if (m_Inventory)
		//	m_Inventory->OnAttack(m_GameObject);
		//if (m_TemporaryEffects)
		//	m_TemporaryEffects->OnAttack(m_GameObject);

		return true;
	}

	return false;
}

void NetPlayerComponent::ExitState(PlayerStates newState)
{
	switch (m_CurrState)
	{
		case PlayerStates::Idle:
		{
			break;
		}

		case PlayerStates::Move:
		{
			if (newState != PlayerStates::Jump)
				if (m_RigidBody)
					m_RigidBody->SetVelocity(glm::vec3(0, 0, 0));

			if (m_SoundSource) 
			{
				m_SoundSource->Stop();
				m_SoundSource->SetLooping(false);
			}

			break;
		}

		case PlayerStates::Jump:
		{
			break;
		}

		case PlayerStates::Utility:
		{
			m_UtilityTime = 0.0f;
			m_HurtBox->SetEnabled(true);
			break;
		}

		case PlayerStates::Defensive:
		{
			break;
		}

		case PlayerStates::PrimaryAttack_Combo1:
		{
			if (m_WeaponCollider)
			{
				m_WeaponCollider->SetEnabled(false);
				m_comboBreaker = true;
				m_hitManager->ResetObjects();
			}

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::PrimaryAttack_Combo2:
		{
			m_WeaponCollider->SetEnabled(false);
			m_comboBreaker = true;
			m_hitManager->ResetObjects();

			if (!m_Server && (*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				RotateToCamera();

			break;
		}

		case PlayerStates::SecondaryAttack:
		{
			m_WeaponCollider->SetEnabled(false);
			m_Weapon->GetComponent<HitManager>()->ResetObjects();
			break;
		}

		case PlayerStates::TertiaryAttack:
		{
			m_WeaponCollider->SetEnabled(false);
			m_hitManager->ResetObjects();
			break;
		}

		case PlayerStates::SuperAbility:
		{
			m_WeaponCollider->SetEnabled(false);
			m_hitManager->ResetObjects();
			break;
		}
	}
}

#pragma endregion

void NetPlayerComponent::OnStart()
{
	size_t index;

	// Cache Components
	m_Stats = m_GameObject->GetComponent<StatScript>();
	m_Camera = m_GameObject->GetComponent<ThirdPersonCameraComponent>();
	m_RigidBody = m_GameObject->GetComponent<RigidBodyComponent>();
	m_HurtBox = m_GameObject->GetComponent<CapsuleCollider>();
	DataManager* dataManager = DataManager::GetInstance();

	// Model And Animation Loading.
#if MixamoWarrior
	index = dataManager->LoadMesh("/Player-Warrior/warrior.fbx");
	Mesh* mesh = (Mesh*)dataManager->Get(index);

	dataManager->LoadAnimation("/Player-Warrior/run.fbx", index, "run");
	dataManager->LoadAnimation("/Player-Warrior/battlecry.fbx", index, "battlecry");
	dataManager->LoadAnimation("/Player-Warrior/death.fbx", index, "death");
	dataManager->LoadAnimation("/Player-Warrior/idle.fbx", index, "idle");
	dataManager->LoadAnimation("/Player-Warrior/roll.fbx", index, "roll");
	dataManager->LoadAnimation("/Player-Warrior/slash.fbx", index, "slash1");
	dataManager->LoadAnimation("/Player-Warrior/spinattack.fbx", index, "slash2");
	dataManager->LoadAnimation("/Player-Warrior/horzslash.fbx", index, "slash3");
	dataManager->LoadAnimation("/Player-Warrior/fall.fbx", index, "jump");
	dataManager->LoadAnimation("/Player-Warrior/hit.fbx", index, "hit");
	//dm->LoadAnimation("/Player-Warrior/roll.fbx", idx, "roll");
#else
	index = dataManager->LoadMesh("/Player-Base/player.fbx");
	Mesh* mesh = (Mesh*)dataManager->Get(index);
	mesh->m_Texture = DataManager::GetInstance()->LoadTexture("/Player-Base/Adventure.jpg");

	dataManager->LoadAnimation("/Player-Base/idle.fbx", index, Idle_Animation);
	dataManager->LoadAnimation("/Player-Base/slash1.fbx", index, PrimaryAttack_Combo1_Animation);
	dataManager->LoadAnimation("/Player-Base/slash2.fbx", index, PrimaryAttack_Combo2_Animation);
	dataManager->LoadAnimation("/Player-Base/secondary.fbx", index, SecondaryAttack_Animation);
	dataManager->LoadAnimation("/Player-Base/tertiary.fbx", index, TertiaryAttack_Animation);
	dataManager->LoadAnimation("/Player-Base/super.fbx", index, SuperAbility_Animation);
	dataManager->LoadAnimation("/Player-Base/defensive.fbx", index, Defensive_Animation);
	dataManager->LoadAnimation("/Player-Base/roll.fbx", index, Utility_Animation);
	dataManager->LoadAnimation("/Player-Base/run.fbx", index, Walk_Animation);
	dataManager->LoadAnimation("/Player-Base/great_sword_strafe_inPlace.fbx", index, StrafeLeft_Animtion);
	dataManager->LoadAnimation("/Player-Base/great_sword_strafe_inPlace_1.fbx", index, StrafeRight_Animtion);
	dataManager->LoadAnimation("/Player-Base/jump.fbx", index, Jump_Animation);
	dataManager->LoadAnimation("/Player-Base/stun.fbx", index, HitStun_Animation);
	dataManager->LoadAnimation("/Player-Base/death.fbx", index, Death_Animation);
#endif

	// Object Creation
	m_Inventory = new InventorySubComponent();
	m_TemporaryEffects = new TimedEffectsSubComponent();
	m_Weapon = new Object();
	m_WeaponCollider = new CapsuleCollider(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), 0.5f, ColliderTag::HitBox, true, false);
	m_lighting = new LightComponent();
	m_animationComponent = new AnimationComponent(mesh);
	m_weaponMaterialComponent = new MaterialComponent();
	m_hitManager = new HitManager();

	// Set Stats
	//m_Stats->SetAtkSpeed(2.0f);
	//m_Stats->SetCurrentHealth(1000);
	//m_Stats->SetMaxHealth(1000);

	// Inventory and Effects
	//if (m_isServer)
	//	m_Inventory->AddItem((uint16_t)ItemIDs::DiscoSabre, m_isServer, m_GameObject);
	//m_TemporaryEffects->AddEffect(m_GameObject, TimedEffectType::DiscoBuff, m_isServer);

	// Weapon Object
	m_Weapon->m_Transform = glm::mat4(1);
	m_Weapon->tag = Object::ObjectTags::World;
	m_Weapon->m_Creator = m_GameObject->m_Creator;
	m_Weapon->SetParent(m_GameObject);

	// Weapon Object Mesh
	index = dataManager->LoadMesh("Player-Warrior/sword.fbx");
	//index = dataManager->LoadMesh("Player-Warrior/sword.fbx");
	if (index != LOADERROR)
	{
		Mesh* mesh = (Mesh*)dataManager->Get(index);
		//mesh->m_Texture = dataManager->LoadTexture("Sword2_DIFFUSE.jpg");
		m_Weapon->AddComponent(new MeshComponent(mesh));
		m_Weapon->m_Static = false;
	}

	m_Weapon->m_Static = false;

	// Weapon Collider
	m_WeaponCollider->AddOnCollisionExitEvent(PlayerCollisions::WeaponCollisionExit);
	m_WeaponCollider->AddOnCollisionEnterEvent(PlayerCollisions::WeaponCollisionEnter);
	m_WeaponCollider->AddOnCollisionStayEvent(PlayerCollisions::WeaponCollisionStay);
	m_WeaponCollider->SetEnabled(false);

	// Material Component
	m_weaponMaterialComponent->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	// Add Components
	m_Weapon->AddComponent(m_lighting);
	m_Weapon->AddComponent(m_hitManager);
	m_Weapon->AddComponent(m_weaponMaterialComponent);
	m_Weapon->AddComponent(m_WeaponCollider);
	m_GameObject->m_ObjManager->AddObject(m_Weapon);
	m_WeaponCollider->SetStart(glm::vec4(0, 0, 0, 1));//->BuildCapsuleAroundMesh();
	m_WeaponCollider->SetEnd(glm::vec4(0, -1.5f, 0, 1));
	m_WeaponCollider->SetRadius(0.25);

	// Lighting
	m_lighting->SetLightType(LightType::Point);
	m_lighting->SetDirection(glm::vec3(0, 0, 0));
	m_lighting->SetDistance(10);
	m_lighting->SetColor(glm::vec4(1.0f, 0.4f, 0.0f, 1.0f));
	m_lighting->SetOffset(glm::vec3(0, 0, 0));

	// Scaling Shit
	Object* Scaling = new Object();
	//Scaling->m_Transform = glm::scale(glm::mat4(1), glm::vec3(.01f, .01f, .01f));


	m_Animator = m_animationComponent;
	m_Animator->SetAnimation(Idle_Animation);
	m_Animator->SetAnimationState(LoopAnimating);
	m_Animator->SetAnimationSpeed(1.0f);

	Scaling->AddComponent(new MeshComponent(mesh));
	Scaling->AddComponent(m_Animator);

	MaterialComponent* material = new MaterialComponent();
	material->m_Color = glm::vec4(1, 1, 1, -2);
	Scaling->AddComponent(material);
	Scaling->tag = Object::ObjectTags::TagCount;
	//Scaling->m_Parent = m_GameObject;
	Scaling->SetParent(m_GameObject);
	m_GameObject->m_ObjManager->AddObject(Scaling);

	if (m_GameObject->m_Creator != 0 && m_GameObject->m_Creator == (*m_GameObject->m_MyHash))
	{
		size_t healthimage = DataManager::GetInstance()->LoadTexture("health.png");
		size_t health_emptyimage = DataManager::GetInstance()->LoadTexture("health_empty.png");
		GuiImage* emptyHealthbar = new GuiImage(health_emptyimage);
		emptyHealthbar->SetAnchor(GuiElement::BOTTOM);
		emptyHealthbar->SetOffset(0.5f, 1.0f);
		GuiHealthbar* healthbar = new GuiHealthbar(healthimage, m_Stats);
		healthbar->SetAnchor(GuiElement::BOTTOM);
		healthbar->SetOffset(0.5f, 1.0f);
		Canvas* canvas = m_GameObject->m_ObjManager->GetCanvas();

		canvas->AddElement(emptyHealthbar);
		canvas->AddElement(healthbar);
	}

	if (!m_isServer)
	{
		m_SoundSource = new SoundSourceComponent();
		m_GameObject->AddComponent(m_SoundSource);
	}
	// Projectile test stuff here and possible future projectile stuff also
	for (int i = 0; i < 10; i++)
	{
		Projectile * temp = new Projectile;
		temp->m_ObjManager = m_GameObject->m_ObjManager;
		temp->SetIsPlayer(true);
		temp->SetEnabled(false);
		temp->m_Static = false;
		//temp->SetParent(m_GameObject);
		temp->SetOwner(m_GameObject);
		temp->m_Persistent = true;
		temp->m_Creator = m_GameObject->m_Creator;

		SphereCollider * pc = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::HitBox, true, false);

		pc->AddOnCollisionEnterEvent(PlayerCollisions::ProjectileCollisionEnter);
		pc->AddOnCollisionStayEvent(PlayerCollisions::WeaponCollisionStay);
		pc->AddOnCollisionExitEvent(PlayerCollisions::WeaponCollisionExit);
		pc->SetEnabled(true);

		temp->AddComponent(new HitManager);
		temp->AddComponent(pc);
		proj.push_back(temp);
		m_GameObject->m_ObjManager->AddObject(temp);
	}

	//tempProj = new Projectile;
	//tempProj->m_ObjManager = m_GameObject->m_ObjManager;
	//tempProj->SetIsPlayer(true);
	//tempProj->SetEnabled(false);
	//tempProj->m_Static = false;
	//tempProj->m_Persistent = true;
	//m_GameObject->m_ObjManager->AddObject(tempProj);
}

void NetPlayerComponent::SetMoveSpeed(float f) { m_Stats->SetMoveSpeed(f); }

void NetPlayerComponent::OnUpdate(float deltaTime)
{
	if (m_isServer)
	{
		ServerUpdate(deltaTime);
	}
	else
	{
		//float hp = m_Stats->GetCurrentHealth();

		//char data[32];
		//_itoa_s((int)hp, data, 10);
		//debug
		//printf("PRINTING : %f %s\n", hp, data);
		//Renderer::GetInstance()->RenderText(DKEngine::GetInstance()->GetFont(), data, .5f, 0.5f);

		ClientUpdate(deltaTime);
	}

	m_Inventory->OnUpdate(m_GameObject, deltaTime);
	m_TemporaryEffects->OnUpdate(m_GameObject, deltaTime);

	//Hack: Debug Code
	if (m_GameObject->m_Transform[3][1] <= 0.0f)
		m_GameObject->m_Transform[3][1] = 0.0f;
	if (m_Input->IsKeyDown('L'))
	{
		m_GameObject->m_Transform[3] = glm::vec4(-2.0f, 0.0f, -2.0f, 1.0f);
	}
	//End Debug Code

	glm::mat4 stuff = (*m_Animator->GetInterpolatedMatrixByName("RightHand"));
	m_Weapon->m_Transform[0] = stuff[0];
	m_Weapon->m_Transform[1] = stuff[1];
	m_Weapon->m_Transform[2] = stuff[2];
	//m_Weapon->m_Transform[3] = glm::scale(glm::mat4(1), glm::vec3(0.01f)) * stuff[3];
	m_Weapon->m_Transform[3] = stuff[3];
	m_Weapon->m_Transform = glm::rotate(m_Weapon->m_Transform, -90.0f * glm::pi<float>() / 180.0f, glm::vec3(0, 0, -1));
	m_Weapon->m_Transform = m_Weapon->m_Transform * glm::translate(glm::mat4(1), glm::vec3(0.12f, 0.0f, 0.0f));
	//m_Weapon->m_Transform = glm::translate(m_Weapon->m_Transform, glm::vec3(0.0f, 0.0f, .2f));

	if (m_GameObject->m_Transform[3].y < 0.0f)
		m_GameObject->m_Transform[3].y = 0;

	PacketUpdate(deltaTime);
}

void NetPlayerComponent::ServerUpdate(const float & deltaTime)
{
	//printf("SUPDATE\n");
	//printf("SUPDATE: %f %f %f\n", this->m_GameObject->m_Transform[3][0], this->m_GameObject->m_Transform[3][1], this->m_GameObject->m_Transform[3][2]);

	HandleState(deltaTime);
	//printf("Player Health on Server: %f\n", m_Stats->GetCurrentHealth());
	//printf("SERVER State: %d\n", m_CurrState);
}

void NetPlayerComponent::ClientUpdate(const float & deltaTime)
{
	//printf("CUPDATE: %f %f %f\n", this->m_GameObject->m_Transform[3][0], this->m_GameObject->m_Transform[3][1], this->m_GameObject->m_Transform[3][2]);
	HandleState(deltaTime);
	//printf("CLIENT State: %d\n", m_CurrState);
}

void NetPlayerComponent::HandleMessage(RakNet::BitStream * bs)
{
	MessageTypes type;
	bs->Read<MessageTypes>(type);
	//printf("%i\n", type);

	switch (type)
	{
		case BaseNet_Comp::MessageTypes::Transform:
		{
			glm::mat4x4 mat;
			bs->Read<glm::mat4x4>(mat);
			//if ((m_GameObject->m_Transform[3] - mat[3]).length() < 3)
			//{
			//auto vec = glm::mix(m_GameObject->m_Transform[3], mat[3], .25f);
			//m_GameObject->m_Transform[3] = vec;
			//}
			m_GameObject->m_Transform = mat;
			//printf("Trans update: %f %f %f\n", m_GameObject->m_Transform[3][0], m_GameObject->m_Transform[3][1], m_GameObject->m_Transform[3][2]);
			break;
		}

		case MessageTypes::StateChanged:
		{

			PlayerStates temp;
			bs->Read<PlayerStates>(temp);
			SetState(temp);
			break;
		}
		case MessageTypes::ForwardVector:
		{

			glm::vec4 forward;
			bs->Read<glm::vec4>(forward);
			const glm::vec3 up(0, 1, 0);
			glm::vec3 x = glm::cross(up, glm::vec3(forward[0], forward[1], forward[2]));
			//x = normalize(x);

			m_GameObject->m_Transform[0].x = x.x;
			m_GameObject->m_Transform[0].y = x.y;
			m_GameObject->m_Transform[0].z = x.z;

			m_GameObject->m_Transform[1].x = up.x;
			m_GameObject->m_Transform[1].y = up.y;
			m_GameObject->m_Transform[1].z = up.z;

			m_GameObject->m_Transform[2].x = forward.x;
			m_GameObject->m_Transform[2].y = forward.y;
			m_GameObject->m_Transform[2].z = forward.z;
			//printf("SERVER MAT FORWARD: %f %f %f\n", m_GameObject->m_Transform[2].x, m_GameObject->m_Transform[2].y, m_GameObject->m_Transform[2].z);

			break;
		}
		case MessageTypes::InventoryUpdate:
		{

			uint64_t numMessages = 0;
			bs->Read<uint64_t>(numMessages);
			//printf("NumInventoryMessages: %i, isServer: %i\n", numMessages, m_isServer);
			InventorySubComponent::InventoryMessage message;
			for (uint64_t i = 0; i < numMessages; ++i)
			{
				bs->Read<uint16_t>(message.m_Type);
				bs->Read<uint16_t>(message.m_ManagerIndex);
				//bs->Read<uint64_t>(message.m_Index);

				if ((InventorySubComponent::MessageTypes)message.m_Type == InventorySubComponent::MessageTypes::InventoryAdded)
				{
					m_Inventory->AddItem(message.m_ManagerIndex, m_isServer, m_GameObject);
				}
				else if ((InventorySubComponent::MessageTypes)message.m_Type == InventorySubComponent::MessageTypes::InventoryRemoved)
				{
					m_Inventory->RemovePickups(m_isServer, m_GameObject);
				}
			}

			break;
		}
		case MessageTypes::EffectsUpdate:
		{

			uint64_t numMessages = 0;
			bs->Read<uint64_t>(numMessages);
			//printf("NumInventoryMessages: %i, isServer: %i\n", numMessages, m_isServer);
			TimedEffectsSubComponent::NetworkMessage message;
			for (int i = 0; i < numMessages; ++i)
			{
				bs->Read<uint16_t>(message.m_MessageType);
				bs->Read<uint16_t>(message.m_EffectType);

				if (message.m_MessageType == MessageTypes::EffectsAdded)
				{
					m_TemporaryEffects->AddEffect(m_GameObject, (TimedEffectType)message.m_EffectType, m_isServer);
				}
				else if (message.m_MessageType == MessageTypes::EffectsCleared)
				{
					m_TemporaryEffects->ClearEffects(m_GameObject, m_isServer);
				}
			}
		}
		case BaseNet_Comp::MessageTypes::Health:
		{

			float nhp;
			bs->Read<float>(nhp);
			if (m_Stats)
				m_Stats->SetCurrentHealth(nhp);
			break;
		}
		case BaseNet_Comp::MessageTypes::Physics:
		{
			glm::vec3 temp;
			bs->Read<glm::vec3>(temp);
			m_GameObject->GetComponent<RigidBodyComponent>()->SetVelocity(temp);
			//printf("NET VEL update: %f %f %f\n", temp.x, temp.y, temp.z);

		}break;
		default:
		{

			break;
		}
	}
}

void NetPlayerComponent::PacketUpdate(const float & deltaTime)
{
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
				bs.Write<float>(m_Stats->GetCurrentHealth());
				m_Server->SendObjPacket(&bs);
			}
			//{
			//	RakNet::BitStream bs;
			//	bs.Write(GetNetworkID());
			//	bs.Write(MessageTypes::StateChanged);
			//	bs.Write<PlayerStates>(m_CurrState);
			//	m_Server->SendObjPacket(&bs);
			//}

			if (m_SyncVelocity)
			{
				if (m_RigidBody)
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write(BaseNet_Comp::MessageTypes::Physics);
					bs.Write<glm::vec3>(m_RigidBody->GetVelocity());
					m_Server->SendObjPacket(&bs);
				}
			}

			if (m_Inventory)
			{
				std::vector<InventorySubComponent::InventoryMessage>& InventoryMessages = m_Inventory->getMessageBacklog();
				if (InventoryMessages.size() >= 1)
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write<uint16_t>((uint16_t)MessageTypes::InventoryUpdate);
					bs.Write<uint64_t>((uint64_t)InventoryMessages.size());
					for (auto& message : InventoryMessages)
					{
						bs.Write<uint16_t>(message.m_Type);
						bs.Write<uint16_t>(message.m_ManagerIndex);
						//bs.Write<uint64_t>(message.m_Index);
					}
					m_Server->SendObjPacket(&bs);
					InventoryMessages.clear();
				}
			}

			if (m_TemporaryEffects)
			{
				auto& EffectMessages = m_TemporaryEffects->GetMessageBacklog();
				if (EffectMessages.size() >= 1)
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write<uint16_t>((uint16_t)MessageTypes::EffectsUpdate);
					bs.Write<uint64_t>((uint64_t)EffectMessages.size());
					for (auto& message : EffectMessages)
					{
						bs.Write<uint16_t>(message.m_MessageType);
						bs.Write<uint16_t>(message.m_EffectType);
					}
					m_Server->SendObjPacket(&bs);
					EffectMessages.clear();
				}
			}

			elpsTime -= m_SyncRate;
		}
	}
	else
	{
		elpsTime += deltaTime;
		if (this->m_SyncRate < elpsTime)
		{
			//send forward vetor to the server
			if (m_SyncForwardVec)
			{
				if ((*m_GameObject->m_MyHash) == m_GameObject->m_Creator)
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write(MessageTypes::ForwardVector);
					bs.Write<glm::vec4>(m_GameObject->m_Transform[2]);
					m_Client->SendObjPacket(&bs);
				}
			}

			elpsTime -= m_SyncRate;
		}
	}
}

PlayerStates NetPlayerComponent::GetState()
{
	return m_CurrState;
}

Object * NetPlayerComponent::GetWeapon()
{
	return m_Weapon;
}

void NetPlayerComponent::SetState(PlayerStates s)
{
	if (s != m_CurrState)
	{
		//if(m_isServer)
		//	printf("Server STATE: %i\n", m_CurrState);
		//else
		//	printf("Client STATE: %i\n", m_CurrState);

		//m_PrevState = m_CurrState;
		ExitState(s);
		m_CurrState = s;

		if (m_isServer)
		{
			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(MessageTypes::StateChanged);
			bs.Write<PlayerStates>(m_CurrState);
			m_Server->SendObjPacket(&bs, RELIABLE_ORDERED);
		}

		switch (s)
		{
			case PlayerStates::Idle:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(Idle_Animation);
					m_Animator->SetAnimationState(LoopAnimating);
					m_Animator->SetAnimationSpeed(m_Stats->GetMoveSpeedModifier());
				}
				break;
			}
			case PlayerStates::Move:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(Walk_Animation);
					m_Animator->SetAnimationState(LoopAnimating);
					m_Animator->SetAnimationSpeed(m_Stats->GetMoveSpeedModifier());
				}

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(Walk_Sound));
					m_SoundSource->SetLooping(true);
					//m_SoundSource->Play();
				}

				break;
			}

			case PlayerStates::Jump:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(Jump_Animation);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed(1.0f);
				}

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(Jump_Sound));
					m_SoundSource->Play();
				}

				break;
			}

			case PlayerStates::Utility:
			{
				m_UtilityTime = 0.0f;
				m_Animator->SetAnimation(Utility_Animation);
				m_Animator->SetAnimationState(SingleAnimating);
				m_Animator->SetAnimationSpeed(m_Animator->GetCurrentTotalAniDuration() / m_UtilityDuration);

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(UtilityAbility_Sound));
					m_SoundSource->Play();
				}

				m_HurtBox->SetEnabled(false);

				break;
			}

			case PlayerStates::Defensive:
			{
				m_DefensiveTime = 0.0f;
				m_Animator->SetAnimation(Defensive_Animation);
				m_Animator->SetAnimationState(SingleAnimating);
				m_Animator->SetAnimationSpeed(m_Animator->GetCurrentTotalAniDuration() / m_UtilityDuration);

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(DefensiveAbility_Sound));
					m_SoundSource->Play();
				}

				break;
			}

			case PlayerStates::PrimaryAttack_Combo1:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(PrimaryAttack_Combo1_Animation);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed((m_Animator->GetCurrentTotalAniDuration() / m_PrimaryAttack_Combo1_Duration) * m_Stats->GetAtkSpeed());
				}

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(PrimaryAttack_Combo1_Sound));
					m_SoundSource->Play();
				}

				// proj stufff
				//FireProjectile(-1.0f);
				/*if (!proj[projItr]->isEnabled())
				{
					proj[projItr]->SetEnabled(true);
					proj[projItr]->GetComponent<SphereCollider>()->SetEnabled(true);
					proj[projItr]->m_Transform = m_GameObject->m_Transform;
					proj[projItr]->m_Transform[3][1] += 0.5f;
					proj[projItr]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);

					Utils::PrintWarning("Projectile info: #: %d, Collider Enabled: %s\n",
						projItr,
						(proj[projItr]->GetComponent<SphereCollider>()->IsEnabled()) ? "True" : "False");

					projItr++;
				}
				if (projItr >= 10)
					projItr = 0;*/

				/*if (!tempProj->isEnabled())
				{
					tempProj->SetEnabled(true);
					tempProj->m_Transform = m_GameObject->m_Transform;
					tempProj->m_Transform[3][1] += 1.0f;
					tempProj->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
				}*/

				m_comboBreaker = true;
				//m_Weapon->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
				m_PrimaryAttack_Combo1_Time = 0.0f;
				m_WeaponCollider->SetEnabled(true);

				if (m_Inventory)
					m_Inventory->OnAttack(m_GameObject);
				if (m_TemporaryEffects)
					m_TemporaryEffects->OnAttack(m_GameObject);

				break;
			}

			case PlayerStates::PrimaryAttack_Combo2:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(PrimaryAttack_Combo2_Animation);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed((m_Animator->GetCurrentTotalAniDuration() / m_PrimaryAttack_Combo1_Duration) * m_Stats->GetAtkSpeed());
				}

				//FireProjectile(-1.0f);
				/*if (!proj[projItr]->isEnabled())
				{
					proj[projItr]->SetEnabled(true);
					proj[projItr]->GetComponent<SphereCollider>()->SetEnabled(true);
					proj[projItr]->m_Transform = m_GameObject->m_Transform;
					proj[projItr]->m_Transform[3][1] += 1.0f;
					proj[projItr]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
					projItr++;
				}
				if (projItr >= 10)
					projItr = 0;*/

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(PrimaryAttack_Combo2_Sound));
					m_SoundSource->Play();
				}

				if (m_Inventory)
					m_Inventory->OnAttack(m_GameObject);
				if (m_TemporaryEffects)
					m_TemporaryEffects->OnAttack(m_GameObject);

				m_comboBreaker = true;
				//m_Weapon->GetComponent<MaterialComponent>()->SetColor(glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
				m_PrimaryAttack_Combo2_Time = 0.0f;
				m_WeaponCollider->SetEnabled(true);

				break;
			}

			case PlayerStates::SecondaryAttack:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(SecondaryAttack_Animation, 0.0f);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed((m_Animator->GetCurrentTotalAniDuration() / m_SecondaryAttackDuration) * m_Stats->GetAtkSpeed());
				}

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(SecondaryAttack_Sound));
					m_SoundSource->Play();
				}

				//FireProjectile(-1.0f);
				/*if (!proj[projItr]->isEnabled())
				{
					proj[projItr]->SetEnabled(true);
					proj[projItr]->GetComponent<SphereCollider>()->SetEnabled(true);
					proj[projItr]->m_Transform = m_GameObject->m_Transform;
					proj[projItr]->m_Transform[3][1] += 1.0f;
					proj[projItr]->GetComponent<RigidBodyComponent>()->SetVelocity(m_GameObject->m_Transform[2]);
					projItr++;
				}
				if (projItr >= 10)
					projItr = 0;*/

				m_SecondaryAttackTime = 0.0f;
				m_WeaponCollider->SetEnabled(true);

				break;
			}

			case PlayerStates::TertiaryAttack:
			{
				if (m_Animator)
				{
					if (m_Animator)
					{
						m_Animator->SetAnimation(TertiaryAttack_Animation);
						m_Animator->SetAnimationState(SingleAnimating);
						m_Animator->SetAnimationSpeed((m_Animator->GetCurrentTotalAniDuration() / m_TertiaryAttackDuration) * m_Stats->GetAtkSpeed());
					}


					if (m_SoundSource)
					{
						m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(TertiaryAttack_Sound));
						m_SoundSource->Play();
					}

					m_TertiaryAttackTime = 0.0f;
					m_WeaponCollider->SetEnabled(true);

					break;
				}

			case PlayerStates::SuperAbility:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(SuperAbility_Animation);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed((m_Animator->GetCurrentTotalAniDuration() / m_SuperAbilityDuration) * m_Stats->GetAtkSpeed());
				}

				if (m_SoundSource)
				{
					m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound(SuperAbility_Sound));
					m_SoundSource->Play();
				}

				m_SuperAbilityTime = 0.0f;
				//m_WeaponCollider->SetEnabled(true);

				m_TemporaryEffects->AddEffect(m_GameObject, TimedEffectType::UltBuff, m_isServer);

				break;
			}

			case PlayerStates::Dead:
			{
				if (m_Animator)
				{
					m_Animator->SetAnimation(Death_Animation);
					m_Animator->SetAnimationState(SingleAnimating);
					m_Animator->SetAnimationSpeed(1.0f);
				}

				break;
			}

			default:
				break;
		}
		}
	}
}

void NetPlayerComponent::RotateToCamera()
		{
			if (m_Camera)
			{
				glm::mat4& cam = m_Camera->GetCamera();
				glm::vec3 zForward = glm::normalize(glm::vec3(cam[2][0], 0, cam[2][2]));
				const glm::vec3 up(0, 1, 0);
				glm::vec3 x = cross(up, zForward);
				//zForward = cross(x, up);
				m_GameObject->m_Transform[0].x = x.x;
				m_GameObject->m_Transform[0].y = x.y;
				m_GameObject->m_Transform[0].z = x.z;

				m_GameObject->m_Transform[1].x = up.x;
				m_GameObject->m_Transform[1].y = up.y;
				m_GameObject->m_Transform[1].z = up.z;

				m_GameObject->m_Transform[2].x = zForward.x;
				m_GameObject->m_Transform[2].y = zForward.y;
				m_GameObject->m_Transform[2].z = zForward.z;
				//m_GameObject->m_Transform = m_GameObject->m_Transform*glm::scale(glm::mat4(1), glm::vec3(.01, .01, .01));

			}
		}

#undef Idle_Animation
#undef PrimaryAttack_Combo1_Animation
#undef PrimaryAttack_Combo2_Animation
#undef SecondaryAttack_Animation
#undef TertiaryAttack_Animation
#undef SuperAbility_Animation			
#undef Defensive_Animation
#undef Utility_Animation
#undef Walk_Animation
#undef StrafeLeft_Animtion
#undef StrafeRight_Animtion
#undef Jump_Animation
#undef HitStun_Animation
#undef Death_Animation