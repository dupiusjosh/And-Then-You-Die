#include "NetEnemy.h"
#include "BaseNet_Comp.h"
#include "DataManager.h"
#include "MeshComponent.h"
#include "NetEnemyComp.h"
#include "AABBCollider.h"
#include "SphereCollider.h"
#include "CollisionDetection.h"
#include "StatScript.h"
#include "RigidBodyComponent.h"
#include "MaterialComponent.h"
#include "StatScriptEvents.h"
#include "NetEnemyRangeComp.h"

NetEnemy::NetEnemy()
{
	tag = Object::Enemy;
	m_Static = false;

	// Set the mesh up for the enemy
	//DataManager * dm = DataManager::GetInstance();
	//
	//size_t idx = dm->LoadMesh("Sphere.fbx", true);


	DataManager * dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("sphere.fbx", true);
	if (idx != LOADERROR)
	{
		Mesh* mesh = (Mesh*)dm->Get(idx);// , Data::DataType::MeshData);
		
		SphereCollider* enemySphereCollider1 = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);
		//SphereCollider* enemySphereCollider2 = new SphereCollider(glm::vec4(-0.0f, 1.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);
		//SphereCollider* enemySphereCollider3 = new SphereCollider(glm::vec4(-0.0f, 2.5f, -0.0f, 1.0f), 0.5f, ColliderTag::Unit, false, true);

		enemySphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
		enemySphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
		enemySphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
		enemySphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);

		//enemySphereCollider2->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider2->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider2->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
		//enemySphereCollider2->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);
		//
		//enemySphereCollider3->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider3->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider3->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
		//enemySphereCollider3->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);

		CapsuleCollider* enemyHurtBox = new CapsuleCollider(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f), glm::vec4(0.0f, 2.0f, 0.0f, 1.0f), 0.5f, ColliderTag::HurtBox);
		
		RigidBodyComponent* playerBody = new RigidBodyComponent();
		playerBody->SetGravity(glm::vec3(0.0f, -9.81f, 0.0f));
		playerBody->SetMass(10.0f);
		MaterialComponent* mc = new MaterialComponent();

		if (rand() % 3 == 0)
		{
			NetEnemyRangeComp * npc = new NetEnemyRangeComp;
			npc->SetSyncRate(0.01f);
			npc->SetSyncTransform(true);
			npc->SetSyncVelocity(true);
			AddComponent(npc);
			//MaterialComponent* mc = new MaterialComponent();
			mc->SetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		}
		else
		{
			NetEnemyComp * npc = new NetEnemyComp;
			npc->SetSyncRate(0.01f);
			npc->SetSyncTransform(true);
			npc->SetSyncVelocity(true);
			AddComponent(npc);
			//MaterialComponent* mc = new MaterialComponent();
			mc->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		StatScript * sc = new StatScript(100.0f, 100.0f, 1.0f, 15.0f, 4.0f);
		sc->AddHealthChangeEvent(HealthChangedEvents::KillUnit);
		sc->AddStunValueChangedEvent(StunEvents::StunEnemy);

		

		AddComponent(sc);
		AddComponent(playerBody);
		
		//AddComponent(new MeshComponent(mesh));
		AddComponent(enemySphereCollider1);
		//AddComponent(enemySphereCollider2);
		//AddComponent(enemySphereCollider3);
		AddComponent(enemyHurtBox);
		AddComponent(mc);
	}

	//RigidBodyComponent* playerBody = new RigidBodyComponent();
	//playerBody->SetGravity(glm::vec3(0.0f, -1.0f, 0.0f));
	//playerBody->SetMass(10.0f);
	//AddComponent(playerBody);

	//if (idx != LOADERROR)
	//{
	//	Mesh* mesh = (Mesh*)dm->Get(idx);// , Data::DataType::MeshData);
	//	AddComponent(new MeshComponent(mesh));
	//}

	//NetEnemyComp * netenem = new NetEnemyComp();
	//netenem->SetSyncRate(.01f);
	//netenem->m_GameObject = this;
	//netenem->SetSyncVelocity(true);
	//AddComponent(netenem);

	//

	//SphereCollider* playerSphereCollider = new SphereCollider(glm::vec4(-0.0f, -0.0f, -0.0f, 1.0f), 0.5f, false,true);
	//playerSphereCollider->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
	//playerSphereCollider->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
	//playerSphereCollider->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);
	//AddComponent(playerSphereCollider);

	////AABBCollider * a = new AABBCollider(glm::vec4(-.5f,-.5f,-.5f,1), glm::vec4(.5f, .5f, .5f, 1),false,true);
	////AddComponent(a);
	//StatScript *stat = new StatScript();
	//stat->SetHealth(50);
	//stat->SetMoveSpeed(2);
	//stat->SetAtkSpeed(50);
	//AddComponent(stat);


	//netenem->SetSyncTransform(true);
}


NetEnemy::~NetEnemy()
{
}
