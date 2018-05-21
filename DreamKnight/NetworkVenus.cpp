#include "NetworkVenus.h"
#include "DataManager.h"
#include "Mesh.h"
#include "MeshComponent.h"

NetworkVenus::NetworkVenus()
{
	//Load the obj
	//Add basenetcomp
	//add mesh rend
	DataManager * dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("venus.fbx", true);

	if (idx != LOADERROR)
	{
		Mesh* mesh = (Mesh*)dm->Get(idx);// , Data::DataType::MeshData);
		AddComponent(new MeshComponent(mesh));
	}

	this->idx = idx;
	//glm::mat4 matrix = m_Transform;
	//m_Transform = glm::transpose(glm::translate(matrix, glm::vec3((rand() - RAND_MAX / 2) % 10, (rand() - RAND_MAX / 2) % 10, (rand() - RAND_MAX / 2) % 10)));
	//m_ObjManager.AddObject(obj);

}


NetworkVenus::~NetworkVenus()
{
}
