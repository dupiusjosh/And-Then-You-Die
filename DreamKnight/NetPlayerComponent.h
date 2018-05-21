#pragma once
#include "BaseNet_Comp.h"
#include "glm.h"
#include "SoundComponent.h"
#include "Projectile.h"
struct CollisionEvent;
class AnimationComponent;
class CapsuleCollider;
class StatScript;
class ThirdPersonCameraComponent;
class RigidBodyComponent;
class InventorySubComponent;
class TimedEffectsSubComponent;
struct MaterialComponent;
class LightComponent;
struct HitManager;
struct MaterialComponent;

enum class PlayerStates : uint16_t
{
	Idle,
	Move,
	Jump,
	Utility,
	Defensive,
	PrimaryAttack_Combo1,
	PrimaryAttack_Combo2,
	SecondaryAttack,
	TertiaryAttack,
	SuperAbility,
	Dead,
	numStates,
	Invalid = 65535,
};

class NetPlayerComponent : public BaseNet_Comp
{
public:
	enum MessageTypes : uint16_t
	{
		StateChanged = BaseNet_Comp::MessageTypes::CustomMessageStart,
		ForwardVector,
		InventoryUpdate,
		InventoryAdded,
		InventoryRemoved,
		EffectsUpdate,
		EffectsAdded,
		EffectsCleared,
	};

private:
	bool m_SyncForwardVec = true;
	// Cached Player Components
	StatScript* m_Stats = nullptr;
	AnimationComponent* m_Animator = nullptr;
	ThirdPersonCameraComponent * m_Camera = nullptr;
	RigidBodyComponent* m_RigidBody = nullptr;
	SoundSourceComponent* m_SoundSource = nullptr;
	MaterialComponent* m_weaponMaterialComponent = nullptr;
	HitManager* m_hitManager = nullptr;
	LightComponent* m_lighting = nullptr;
	AnimationComponent* m_animationComponent = nullptr;
	CapsuleCollider* m_WeaponCollider = nullptr;
	Object* m_Weapon = nullptr;
	CapsuleCollider* m_HurtBox = nullptr;

	std::vector<Projectile*> proj;
	int projItr = 0;
	Projectile * tempProj;

	// Primary Attack Properties
	bool m_comboBreaker = true;

	// Combo 1
	float m_PrimaryAttack_Combo1_Time;
	float m_PrimaryAttack_Combo1_Duration = 0.75f;
	float m_PrimaryAttack_Combo1_DamageModifier = 1.0f;
	float m_PrimaryAttack_Combo1_HitStunValue = 55.0f;
	float m_PrimaryAttack_Combo1_KnockbackValue = 0.0f;
	float m_PrimaryAttack_Combo1_CooldownDuration = 0.0f;
	float m_PrimaryAttack_Combo1_CurrentCooldownTime = 0.0f;
	bool  m_PrimaryAttack_Combo1_CoolingDown = false;

	// Combo 2
	float m_PrimaryAttack_Combo2_Time;
	float m_PrimaryAttack_Combo2_Duration = 0.75f;
	float m_PrimaryAttack_Combo2_DamageModifier = 1.0f;
	float m_PrimaryAttack_Combo2_HitStunValue = 55.0f;
	float m_PrimaryAttack_Combo2_KnockbackValue = 0.0f;
	float m_PrimaryAttack_Combo2_CooldownDuration = 0.0f;
	float m_PrimaryAttack_Combo2_CurrentCooldownTime = 0.0f;
	bool  m_PrimaryAttack_Combo2_CoolingDown = false;

	// Secondary Attack Properties
	float m_SecondaryAttackTime;
	float m_SecondaryAttackDuration = 1.5f;
	float m_SecondaryAttack_DamageModifier = 2.0f;
	float m_SecondaryAttack_HitStunValue = 1000.0f;
	float m_SecondaryAttack_KnockbackValue = 0.0f;
	float m_SecondaryAttack_CooldownDuration = 5.0f;
	float m_SecondaryAttack_CurrentCooldownTime = 0.0f;
	bool  m_SecondaryAttack_CoolingDown = false;

	// Tertiary Attack Properties
	float m_TertiaryAttackTime;
	float m_TertiaryAttackDuration = 1.0f;
	float m_TertiaryAttack_DamageModifier = 3.0f;
	float m_TertiaryAttack_HitStunValue = 1000.0f;
	float m_TertiaryAttack_KnockbackValue = 50.0f;
	float m_TertiaryAttack_CooldownDuration = 10.0f;
	float m_TertiaryAttack_CurrentCooldownTime = 0.0f;
	bool  m_TertiaryAttack_CoolingDown = false;

	// Super Ability Properties
	float m_SuperAbilityTime;
	float m_SuperAbilityDuration = 1.0f;
	float m_SuperAbility_DamageModifier = 5.0f;
	float m_SuperAbility_HitStunValue = 0.0f;
	float m_SuperAbility_KnockbackValue = 0.0f;
	float m_SuperAbility_CooldownDuration = 0.0f;
	float m_SuperAbility_CurrentCooldownTime = 0.0f;
	bool  m_SuperAbility_CoolingDown = false;

	// Defensive Properties
	float m_DefensiveTime;
	float m_DefensiveDuration;

	// Functions for handling player states 
	void HandleState(float deltaTime);
	bool HandleMovement(bool allowStateChange = true);
	bool HandleJump();
	bool HandleUtilityAbility();
	bool HandleDefensiveAbility();
	bool HandlePrimaryAttack();
	bool HandleSecondaryAttack();
	bool HandleTertiaryAttack();
	bool HandleSuperAbility();

	void RotateToCamera();

public:
	NetPlayerComponent();

	float GetDamageModifier();
	float GetDamage();

	float GetStunValue();

	//Public Cached player components
	InventorySubComponent* m_Inventory;
	TimedEffectsSubComponent* m_TemporaryEffects;

	PlayerStates m_CurrState= PlayerStates::Invalid;

	// Charge Properties
	float m_UtilityTime;
	float m_UtilityDuration = 0.5f;
	float m_UtilitySpeed = 7.5f;

	void OnStart();
	void OnUpdate(float deltaTime);
	void ServerUpdate(const float& deltaTime);
	void ClientUpdate(const float& deltaTime);
	void PacketUpdate(const float& deltaTime);
	void HandleMessage(RakNet::BitStream *bs) override;

	// Getters
	PlayerStates GetState();
	Object* GetWeapon();

	// Setters
	void SetMoveSpeed(float f);
	void SetState(PlayerStates s);
	void ExitState(PlayerStates newState);

	//Other
	void FireProjectile(float Damage = -1.0f);

	~NetPlayerComponent();
	void HandleCoolDowns(float deltaTime);
};