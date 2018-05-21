#include "Chest.h"
#include "DataManager.h"
#include "MeshComponent.h"
#include "NetChestComponent.h"
#include "CollisionDetection.h"
#include "AnimationComponent.h"

Chest::Chest()
{
	m_Static = false;
	DataManager* dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("TChest.fbx");
	size_t idxT = dm->LoadTexture("Treasure_chest_UV.png");
	Mesh* m = nullptr;
	//m_Transform = glm::scale(glm::mat4(1), glm::vec3(0.1, 0.1, 0.1));
	if (idx != LOADERROR)
	{
		Mesh* m = (Mesh*)dm->Get(idx, Data::DataType::MeshData);

		if (idxT != LOADERROR)
		{
			m->m_Texture = idxT;
		}

		AddComponent(new MeshComponent(m));
		size_t idx2 = dm->LoadAnimation("TChest.fbx", idx, "open");
		if (idx2 != LOADERROR)
		{
			AnimationComponent* ac = new AnimationComponent(m);
			AddComponent(ac);
			ac->SetAnimation("open");
			ac->SetAnimationState(NotAnimating);
			//ac->SetDebug(true);
			ac->SetAnimationSpeed(0.3f);
		}
	}
	NetChestComp * nCc = new NetChestComp();
	nCc->SetSyncRate(1.0f);
	AddComponent(nCc);
	tag = ObjectTags::Chest;

	SphereCollider* collinder = new SphereCollider(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f), 0.5f, ColliderTag::HurtBox, false, false);
	//CapsuleCollider* collinder = new CapsuleCollider(glm::vec4(0.5f, 0.5f, 0.0f, 1.0f), glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f), 0.5f, ColliderTag::HurtBox, false, false);
	collinder->AddOnCollisionEnterEvent(CollisionResponse::PushOut_SphereToSphere);
	AddComponent(collinder);

}
