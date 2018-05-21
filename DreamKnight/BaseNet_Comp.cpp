#include "BaseNet_Comp.h"
#include "glm.h"
#include "NetClient.h"
#include "NetServer.h"
#include "InputManager.h"
#include "RigidBodyComponent.h"

BaseNet_Comp::BaseNet_Comp()
{
}

BaseNet_Comp::~BaseNet_Comp()
{

}

void BaseNet_Comp::OnUpdate(float deltaTime)
{
	if (m_isServer)
	{	
		ServerUpdate(deltaTime);
	}
	else
	{
		ClientUpdate(deltaTime);
	}
}

void BaseNet_Comp::PacketUpdate(const float& deltaTime)
{
	elpsTime += deltaTime;
	if (this->m_SyncRate < elpsTime)
	{
		if (m_ServSendTransform && m_isServer)
		{

			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(BaseNet_Comp::MessageTypes::Transform);
			bs.Write<glm::mat4x4>(m_GameObject->m_Transform);
			m_Server->SendObjPacket(&bs);

		}
		if(m_isServer)
		{
			StatScript *stats = m_GameObject->GetComponent<StatScript>();
			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(BaseNet_Comp::MessageTypes::Health);
			bs.Write<float>(stats->GetCurrentHealth());
			m_Server->SendObjPacket(&bs);
		}

		if (m_SyncVelocity && m_isServer)
		{
			RigidBodyComponent * rig = m_GameObject->GetComponent<RigidBodyComponent>();
			if (rig)
			{
				RakNet::BitStream bs;
				bs.Write(GetNetworkID());
				bs.Write(MessageTypes::Physics);
				bs.Write<glm::vec3>(rig->GetVelocity());
				m_Server->SendObjPacket(&bs);
			}
		}
		elpsTime -= m_SyncRate;
	}
}

void BaseNet_Comp::ServerUpdate(const float & deltaTime)
{
	//if (m_ServSendTransform)
	//{
	//	RakNet::BitStream bs;
	//	bs.Write(GetNetworkID());
	//	bs.Write(MessageTypes::Transform);
	//	bs.Write<glm::mat4x4>(m_GameObject->m_Transform);
	//	m_Server->SendObjPacket(&bs);
	//}
}

void BaseNet_Comp::ClientUpdate(const float & deltaTime)
{
	//No transform
}

void BaseNet_Comp::HandleMessage(RakNet::BitStream * bs)
{

	MessageTypes type;
	bs->Read<MessageTypes>(type);
	//bs->IgnoreBytes(sizeof(MessageTypes));
	//printf("%i\n", type);
	switch (type)
	{
	case BaseNet_Comp::Transform:
	{
		glm::mat4x4 mat;
		bs->Read<glm::mat4x4>(mat);

#ifdef _DEBUG
		//std::cout << "Custom transform packet!\n";
		//std::cout << mat[0].x << ' ' << mat[0].y << ' ' << mat[0].z << ' ' << mat[0].w << '\n'
		//	<< mat[1].x << ' ' << mat[1].y << ' ' << mat[1].z << ' ' << mat[1].w << '\n'
		//	<< mat[2].x << ' ' << mat[2].y << ' ' << mat[2].z << ' ' << mat[2].w << '\n'
		//	<< mat[3].x << ' ' << mat[3].y << ' ' << mat[3].z << ' ' << mat[3].w << '\n';
#endif // _DEBUG
		//printf("TRANSFORUPDATE");
		//auto vec = glm::lerp(m_GameObject->m_Transform[3], mat[3], .1f);
		//m_GameObject->m_Transform[3] = vec;
		m_GameObject->m_Transform = mat;
		break;
	}
	case BaseNet_Comp::Physics:
	{
		RigidBodyComponent * rig = m_GameObject->GetComponent<RigidBodyComponent>();
		if (rig)
		{
			glm::vec3 data;
			bs->Read<glm::vec3>(data);
			printf("Set Velocity to (%f, %f, %f)\n", data.x, data.y, data.z);
			rig->SetVelocity(data);
		}
		break;
	}
	case BaseNet_Comp::MessageTypes::Health:
	{
		StatScript * stats = m_GameObject->GetComponent<StatScript>();
		if (stats)
		{
			float nhp;
			bs->Read<float>(nhp);
			stats->SetCurrentHealth(nhp);
		}
	}
	default:
		break;
	}
}
