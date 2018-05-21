#include "Pickup.h"
#include "DataManager.h"
#include "MeshComponent.h"
#include "SphereCollider.h"
#include "ObjectManager.h"
#include "PickupControlComponent.h"
#include "NetPlayerComponent.h"
#include "InventorySubComponent.h"
#include "TimedEffectsSubComponent.h"
#include "StatScript.h"
#include "ItemRef.h"
#include "Items.h"
#include "MaterialComponent.h"

void Pickup::Start()
{
	if (m_Activated)
	{
		SetEnabled(true);
	}
}

void Pickup::Update(float deltaTime)
{
	Object::Update(deltaTime);
	m_Transform = glm::rotate(m_Transform, glm::half_pi<float>() * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
	auto render = Renderer::GetInstance();
	if (!m_Controller->IsServer())
	{
		auto collinder = GetComponent<SphereCollider>();
		glm::mat4 transform = glm::translate(glm::mat4(), glm::vec3(collinder->GetWorldPosition()));
		transform = glm::scale(transform, glm::vec3(0.2f, 0.2f, 0.2f));
		//render->RenderMesh(/*m_MeshComp->GetMesh());*/(Mesh*)DataManager::GetInstance()->Get(DataManager::GetInstance()->LoadMesh("Player-Warrior/sword.fbx")), m_Transform);
	}
}

Pickup::Pickup() : Object()
{
	m_Transform = glm::translate(glm::mat4(), glm::vec3(4.0, 12.0, 4.0)) * glm::scale(glm::mat4(), glm::vec3(0.2f, 0.2f, 0.2f));
	tag = ObjectTags::Pickup;
	m_Enable = false;
	m_Activated = false;
	m_Controller = new PickupNetworkComponent();
	m_Controller->m_ControlledPickup = this;
	AddComponent(m_Controller);
}

Pickup::~Pickup()
{
}

static void OnCollisionHealth(CollisionEvent e)
{
	//Debug Score
	StatScript* player = e.m_CollidingWith->m_GameObject->GetComponent<StatScript>();
	Pickup* thisObject = (Pickup*)e.m_ThisObject->m_GameObject;

	if (thisObject->m_Collected == false && e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Player)
	{
		//if (thisObject->m_Controller->IsServer() == false)
		//{
		//	printf("CLIENT: HealthPickupCollected. Previously collected: %s\n", (thisObject->m_Collected) ? "yes" : "no");
		//}
		if (player)
		{
			thisObject->m_Collected = true;
			player->ModifyHealth(thisObject->m_Value.m_HealthValue);
			thisObject->Deactivate();
		}
	}

}

static void OnCollisionItem(CollisionEvent e)
{
	NetPlayerComponent* player = e.m_CollidingWith->m_GameObject->GetComponent<NetPlayerComponent>();
	Pickup* thisObject = (Pickup*)e.m_ThisObject->m_GameObject;

	if (thisObject->m_Collected == false && e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Player)
	{
		thisObject->m_Collected = true;
		if (player)
		{
			player->m_Inventory->AddItem(thisObject->m_Value.m_ItemIndex, player->IsServer(), player->m_GameObject);
		}
		thisObject->Deactivate();
	}
}

static void OnCollisionEffect(CollisionEvent e)
{
	NetPlayerComponent* player = e.m_CollidingWith->m_GameObject->GetComponent<NetPlayerComponent>();
	Pickup* thisObject = (Pickup*)e.m_ThisObject->m_GameObject;

	if (thisObject->m_Collected == false && e.m_CollidingWith->m_GameObject->tag == Object::ObjectTags::Player)
	{
		thisObject->m_Collected = true;
		if (player)
		{
			player->m_TemporaryEffects->AddEffect(player->m_GameObject, (TimedEffectType)thisObject->m_Value.m_ItemIndex, player->IsServer());
		}
		thisObject->Deactivate();
	}
}

void Pickup::Init(char * path, PickupType Type)
{
	DataManager* dm = DataManager::GetInstance();
	AddComponent(new MeshComponent((Mesh*)dm->Get(dm->LoadMesh(path))));
	AddComponent(new MaterialComponent());
	SphereCollider* collider = new SphereCollider(glm::vec4(0, 0, 0, 1), 0.3f, ColliderTag::Other);
	switch (Type)
	{
	case PickupType::HealthPickup:
		collider->AddOnCollisionEnterEvent(OnCollisionHealth);
		break;
	case PickupType::ItemPickup:
		collider->AddOnCollisionEnterEvent(OnCollisionItem);
		break;
	case PickupType::EffectPickup:
		collider->AddOnCollisionEnterEvent(OnCollisionEffect);
		break;
	}
	m_Static = false;
	AddComponent(collider);
	m_Value.m_HealthValue = 0;
	m_Type = Type;
}

void Pickup::Activate(glm::mat4 transform, uint16_t value)
{
	switch (m_Type)
	{
	case (PickupType::HealthPickup): {
		m_Transform = glm::scale(transform, glm::vec3(0.2f, 0.2f, 0.2f));
		break;
	}
	case (PickupType::EffectPickup): {
		m_Transform = glm::scale(transform, glm::vec3(0.6f, 0.6f, 0.6f));
		break;
	}
	case (PickupType::ItemPickup): {
		m_Transform = glm::scale(transform, glm::vec3(0.4f, 0.4f, 0.4f));
		break;
	}
	default: {
		m_Transform = glm::scale(transform, glm::vec3(0.6f, 0.6f, 0.6f));
		break;
	}
	}

	m_Transform[3][1] = 0.5f;
	m_Value.m_HealthValue = value;// 100;
	m_Collected = false;
	SetMesh(value);
	//m_Transform = transform;
	m_Controller->m_NextMessage.MessageType = PickupNetworkComponent::PickupSpawned;
	m_Controller->m_NextMessage.Transform = m_Transform;
	m_Controller->m_NextMessage.PickupValue = m_Value.m_HealthValue;
	m_Controller->m_MessageAvailable = true;
	m_Activated = true;
	if (m_ObjManager != nullptr)
		SetEnabled(true);
}

void Pickup::Deactivate()
{
	m_Controller->m_NextMessage.MessageType = PickupNetworkComponent::PickupDespawned;
	m_Controller->m_MessageAvailable = true;
	m_Collected = true;
	m_Activated = false;
	SetEnabled(false);
}

void Pickup::SetMesh(uint16_t ItemId)
{
	MeshComponent* MeshComp = GetComponent<MeshComponent>();
	MaterialComponent* MatComp = GetComponent<MaterialComponent>();
	if (MeshComp)
	{
		switch (m_Type)
		{
		case (PickupType::ItemPickup): {
			//ItemRef::GetInstance()->GetItem((ItemIDs)ItemId);
			//MeshComp->SetMesh((Mesh*)DataManager::GetInstance()->Get(DataManager::GetInstance()->LoadMesh("Capsule.fbx")));
			//Item_Base* item = ItemRef::GetItem((ItemIDs)ItemId);
			Item_Base* item = ItemRef::GetItem((ItemIDs)ItemId);
			if (item && item->GetMesh())
				MeshComp->SetMesh(item->GetMesh());
			if (item && MatComp)
				MatComp->m_Color = item->GetColor();
			break;
		}
		case (PickupType::HealthPickup): {
				//ItemRef::GetInstance()->GetItem((ItemIDs)ItemId);
				MeshComp->SetMesh((Mesh*)DataManager::GetInstance()->Get(DataManager::GetInstance()->LoadMesh("HeartPickup.fbx", true)));
				if (MatComp)
					MatComp->m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			break;
		}
		case (PickupType::EffectPickup): {
				//ItemRef::GetInstance()->GetItem((ItemIDs)ItemId);
				//MeshComp->SetMesh((Mesh*)DataManager::GetInstance()->Get(DataManager::GetInstance()->LoadMesh("Capsule.fbx")));
			break;
		}
		}
	}
}

void Pickup::Swap(Pickup * other)
{
	PickupType tempPT = m_Type;
	m_Type = other->m_Type;
	other->m_Type = tempPT;

	void* tempP = m_Controller;
	m_Controller = other->m_Controller;
	other->m_Controller = (PickupNetworkComponent*)tempP;

	uint16_t tempI = m_Value.m_ItemIndex;
	//m_Value.m_ItemIndex = other->m_Value.m_ItemIndex;
	//other->m_Value.m_ItemIndex = tempI;

	if (m_Collected)
	{
		if (!other->m_Collected)
		{
			m_Collected = false;
			other->m_Collected = true;
		}
	}
	else
	{
		if (other->m_Collected)
		{
			m_Collected = true;
			other->m_Collected = false;
		}
	}

}


