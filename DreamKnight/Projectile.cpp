#include "Projectile.h"
#include "BaseNet_Comp.h"
#include "DataManager.h"
#include "SphereCollider.h"
#include "RigidBodyComponent.h"
#include "CollisionDetection.h"
#include "ProjectileComp.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"

Projectile::Projectile() : m_Owner(nullptr)
{
	DataManager * dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("sphere.fbx", true);

	m_Static = false;
	m_Damage = 0.0f;

	if (idx != LOADERROR)
	{
		Mesh* mesh = (Mesh*)dm->Get(idx);

		//SphereCollider* enemySphereCollider1 = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::HitBox, true, false);

		RigidBodyComponent* playerBody = new RigidBodyComponent();
		playerBody->SetGravity(glm::vec3(0.0f, 0.0f, 0.0f));
		playerBody->SetMass(10.0f);

		ProjectileComp * pc = new ProjectileComp(m_IsPlayers);

		MeshComponent * ms = new MeshComponent(mesh);

		MaterialComponent * mc = new MaterialComponent;
		mc->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		AddComponent(playerBody);
		AddComponent(ms);
		//AddComponent(enemySphereCollider1);
		AddComponent(pc);
		AddComponent(mc);
	}
}

Projectile::Projectile(bool _IsPlayers) : m_Owner(nullptr)
{
	m_IsPlayers = _IsPlayers;

	DataManager * dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("sphere.fbx", true);
	m_Transform = glm::mat4(1);

	if (idx != LOADERROR)
	{
		Mesh* mesh = (Mesh*)dm->Get(idx);

		//SphereCollider* enemySphereCollider1 = new SphereCollider(glm::vec4(-0.0f, 0.5f, -0.0f, 1.0f), 0.5f, ColliderTag::HitBox, true, true);

		//enemySphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToMesh);
		//enemySphereCollider1->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
		//enemySphereCollider1->AddOnCollisionStayEvent(CollisionResponse::PushOut_SphereToSphere);

		RigidBodyComponent* playerBody = new RigidBodyComponent();
		playerBody->SetGravity(glm::vec3(0.0f, 0.0f, 0.0f));
		playerBody->SetMass(10.0f);

		ProjectileComp * pc = new ProjectileComp(m_IsPlayers);

		AddComponent(playerBody);
		//AddComponent(enemySphereCollider1);
		AddComponent(pc);
	}
}

Projectile::~Projectile()
{

}

void Projectile::SetDamage(float damage)
{
	m_Damage = damage;
}

float Projectile::GetDamage()
{
	return m_Damage;
}

Object * Projectile::GetOwner()
{
	return m_Owner;
}

void Projectile::SetOwner(Object * owner)
{
	m_Owner = owner;
}

//void Projectile::HitCheck(CollisionEvent e)
//{
//	if (e.m_CollidingWith->m_GameObject->tag == Object::Floor)
//	{
//
//	}
//	else if (m_IsPlayers && e.m_CollidingWith->m_GameObject->tag == Object::Enemy)
//	{
//
//	}
//	else if (!m_IsPlayers && e.m_CollidingWith->m_GameObject->tag == Object::Player)
//	{
//
//	}
//}