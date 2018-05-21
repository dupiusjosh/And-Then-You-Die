#include "NetPlayer.h"
#include "DataManager.h"
#include "BaseNet_Comp.h"
#include "MeshComponent.h"
#include "SphereCollider.h"
#include "NetPlayerComponent.h"
#include "RigidBodyComponent.h"
#include "ThirdPersonCameraComponent.h"
#include "LightComponent.h"
#include "CollisionDetection.h"
#include "StatScript.h"
#include "MaterialComponent.h"
#include "InventorySubComponent.h"
#include "AnimationComponent.h"
#include "StatScriptEvents.h"

//Debug
#include "ItemRef.h"

void FloorCheck(CollisionEvent e)
{
	if (e.m_CollidingWith->m_GameObject->tag == Object::Floor && e.m_ThisObject->m_GameObject->tag == Object::Player && e.m_ThisObject->m_GameObject->GetComponent<NetPlayerComponent>()->GetState() == PlayerStates::Jump)
	{
		e.m_ThisObject->m_GameObject->GetComponent<NetPlayerComponent>()->SetState(PlayerStates::Idle);
	}
}

void ChargeHitCheck(CollisionEvent e)
{
	NetPlayerComponent* npc = e.m_ThisObject->m_GameObject->GetComponent<NetPlayerComponent>();

	if (!npc)
		return;

	if ((e.m_CollidingWith->m_GameObject->tag == Object::Wall || e.m_CollidingWith->m_GameObject->tag == Object::Enemy))
	{
		if (npc->GetState() == PlayerStates::Utility)
		{
			npc->ExitState(npc->GetState());
			npc->SetState(PlayerStates::Idle);
		}
	}
}

NetPlayer::NetPlayer()
{
	tag = Object::Player;

	m_Persistent = true;
	m_Static = false;

	SphereCollider* playerSphereCollider1 = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);
	//SphereCollider* playerSphereCollider2 = new SphereCollider(glm::vec4(-0.0f, 1.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);
	//SphereCollider* playerSphereCollider3 = new SphereCollider(glm::vec4(-0.0f, 2.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);

	playerSphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
	playerSphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
	playerSphereCollider1->AddOnCollisionEnterEvent(FloorCheck);
	playerSphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
	playerSphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);

	//playerSphereCollider2->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider2->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
	//playerSphereCollider2->AddOnCollisionEnterEvent(FloorCheck);
	//playerSphereCollider2->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider2->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);
	//
	//playerSphereCollider3->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider3->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
	//playerSphereCollider3->AddOnCollisionEnterEvent(FloorCheck);
	//playerSphereCollider3->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider3->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);

	CapsuleCollider* playerHurtBox = new CapsuleCollider(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 2.0f, 0.0f, 1.0f), 0.5f, ColliderTag::HurtBox);
	playerHurtBox->AddOnCollisionEnterEvent(ChargeHitCheck);

	RigidBodyComponent* playerBody = new RigidBodyComponent();
	playerBody->SetGravity(glm::vec3(0.0f, -9.81f, 0.0f));
	playerBody->SetMass(10.0f);

	NetPlayerComponent * npc = new NetPlayerComponent;
	npc->SetSyncRate(0.03f);
	npc->SetSyncTransform(true);
	npc->SetSyncVelocity(true);

	StatScript * playerStatScript = new StatScript(100, 100, 1.0f, 25.0f, 5.0f);
	playerStatScript->SetAtkSpeed(1.0f);
	playerStatScript->AddHealthChangeEvent(HealthChangedEvents::KillUnit);
	playerStatScript->AddDamageTakingEvent(DamageTakingEvents::TriggerItems);

	LightComponent* playerLight = new LightComponent();
	playerLight->SetLightType(LightType::Ambient);
	playerLight->SetDirection(glm::vec3(0, 0, 0));
	playerLight->SetDistance(10);
	playerLight->SetColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	playerLight->SetOffset(glm::vec3(0, 1, 0));

	ThirdPersonCameraComponent *cam = new ThirdPersonCameraComponent();

	AddComponent(playerLight);
	AddComponent(playerBody);
	AddComponent(playerHurtBox);
	AddComponent(playerSphereCollider1);
	//AddComponent(playerSphereCollider2);
	//AddComponent(playerSphereCollider3);
	AddComponent(playerStatScript);
	AddComponent(npc);
	AddComponent(cam);
}

NetPlayer::~NetPlayer()
{

}