#include "ThirdPersonCameraComponent.h"
#include "glm.h"
#include "DKEngine.h"
//#include "NetClient.h"
#include "StatScript.h"
#include "NetPlayerComponent.h"

ThirdPersonCameraComponent::ThirdPersonCameraComponent()
{
	m_MoveSpeed = 5.0f;
	//activeIM = im;
	m_activeIM = InputManager::GetInstance();
	//printf("CAMERA INPUT:  %i", m_activeIM);
	//	m_RotXCap = none;
	LookAtOffsetPos = glm::vec3(0, 1, 0);
	m_RotYCap = 75.0f * glm::pi<float>() / 180.0f;
	m_RotYFloor = -5.0f * glm::pi<float>() / 180.0f;
}


ThirdPersonCameraComponent::~ThirdPersonCameraComponent()
{
}

void ThirdPersonCameraComponent::OnUpdate(float deltaTime)
{

	if (m_GameObject->m_Creator != 0 && m_GameObject->m_Creator == (*m_GameObject->m_MyHash) && m_activeIM)// == NetClient::GetInstance()->myHash)
	{
		if (m_freeRoam)
		{
			DebugUpdateMouse(deltaTime);
		}
		else
		{
			
			if (!m_Spectate)
			{
				UpdateMouse(deltaTime);
			}
			else
			{
				ChangeTarget(deltaTime);
				UpdateMouse(deltaTime);
			}
		}

		if (m_activeIM->IsKeyPressed(VK_ESCAPE))
		{
			m_freeRoam = !m_freeRoam;
		}
		if (playerScript->GetState() == PlayerStates::Dead)
		{
			//m_Spectate = true;
		}
		
	}
	//if (m_GameObject->m_Creator != 0)
	//{
		//static glm::vec4 Raypos;
		////if (GetAsyncKeyState(VK_ADD) & 0x1)
		//{
		//	Raypos = m_target->m_Transform[3]; 
		//	Raypos += glm::vec4(0, 1, 0, 0);
		//}
		//glm::vec4 dir = glm::vec4(0);
		////auto quadtree = m_GameObject->m_ObjManager->m_StaticObjectQuadTree;
		//Object* obj;
		//static float h = 0;
		//if (GetAsyncKeyState(VK_ADD))
		//	h += 0.01;
		//if (GetAsyncKeyState(VK_SUBTRACT))
		//	h -= 0.01;
		//if (GetAsyncKeyState(VK_RETURN))
		//	h = 0.0f;
		//if (GetAsyncKeyState(VK_NUMPAD0))
		//{
		//
		//}
		//if (h == 0.0f)
		//{
		//	for (float h = 0; h < 6.28218f; h += 0.025f)
		//	{
		//		dir.x = cos(h);
		//		dir.z = sin(h);
		//		dir = glm::normalize(dir);
		//		glm::vec4 hit = glm::vec4(0);
		//		if (obj = CollisionDetection::RaycastAgainstClosest(Raypos, dir, m_GameObject->m_ObjManager, hit))
		//		{
		//			Renderer::GetInstance()->AddLine(Raypos, hit);
		//			obj->GetMeshColliders()[0]->GetBounds().DrawLines(glm::vec4(1));
		//		}
		//		else
		//		{
		//			Renderer::GetInstance()->AddLine(Raypos, Raypos + dir * 0.1f);
		//		}
		//	}
		//}
		//else
		//{
		//	dir.x = cos(h);
		//	dir.z = sin(h);
		//	dir = glm::normalize(dir);
		//	glm::vec4 hit = glm::vec4(0);
		//	if (obj = CollisionDetection::RaycastAgainstClosest(Raypos, dir, m_GameObject->m_ObjManager, hit))
		//	{
		//		Renderer::GetInstance()->AddLine(Raypos, hit);
		//		obj->GetMeshColliders()[0]->GetBounds().DrawLines(glm::vec4(1));
		//	}
		//	else
		//	{
		//		Renderer::GetInstance()->AddLine(Raypos, Raypos + dir * 0.1f);
		//	}
		//
		//	auto nodes = m_GameObject->m_ObjManager->m_StaticObjectQuadTree->RetrieveNodes(Raypos, dir);
		//	for (auto node : nodes)
		//		node->DrawLines();
		//}

		//quadtree->DrawLines();//
	//}
}

void ThirdPersonCameraComponent::OnStart()
{
	m_render = Renderer::GetInstance();
	//Camera = glm::lookAtLH(glm::vec3(0, 2, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	m_target = m_GameObject;
	CameraOffsetPos.x = 0;
	CameraOffsetPos.y = 0.0f;
	CameraOffsetPos.z = -4;
	m_freeRoam = false;
	//offset = glm::vec3(m_XOffset, m_YOffset, m_ZOffset);
	m_Camera = glm::lookAtLH(CameraOffsetPos, glm::vec3(0, 0, 0), glm::vec3(0, .5f, 0));
	rotY = 0;
	rotX = 0;
	//m_Camera = glm::translate(m_Camera, glm::vec3(0, 0, -m_ZOffset));
	//m_Camera = glm::translate(m_Camera, glm::vec3(m_target->m_Transform[3][0], m_target->m_Transform[3][1], m_target->m_Transform[3][2]));
	this->playerScript = m_target->GetComponent<NetPlayerComponent>();


}
//
//void ThirdPersonCameraComponent::CheckStaticObjects(glm::vec4 point, glm::vec4 direction, ObjectManager* manager)
//{
//	std::vector<Object*> &dynamicObjects = manager->GetDynamicCollidableObjects();
//	std::vector<Object*> &staticObjects = manager->GetStaticObjectsRaycast(point, direction);
//
//	float dist2 = FLT_MAX;
//	Object* object;
//
//	// Dynamic Objects
//	for (unsigned int i = 0; i < dynamicObjects.size(); i++)
//	{
//		auto dynObject = dynamicObjects[i];
//
//
//		auto colliders = dynObject->GetColliders();
//		auto meshColliders = dynObject->GetMeshColliders();
//
//		// Normal Colliders
//		for (size_t j = 0; j < colliders.size(); j++)
//		{
//			if (Raycast(point, direction, colliders[j]) > 0)
//			{
//				glm::vec4 closestPoint = ClosestPointToCollider(point, colliders[j]);
//				float currDist = length2(closestPoint - point);
//
//				if (currDist < dist2)
//				{
//					dist2 = currDist;
//					object = dynObject;
//				}
//			}
//		}
//
//		// Mesh Colliders
//		for (size_t j = 0; j < meshColliders.size(); j++)
//		{
//			if (Raycast(point, direction, &meshColliders[j]->GetBounds()) > 0)
//			{
//				glm::vec4 closestPoint = ClosestPointToCollider(point, &meshColliders[j]->GetBounds());
//				float currDist = length2(closestPoint - point);
//
//				if (currDist < dist2)
//				{
//					dist2 = currDist;
//					object = dynObject;
//				}
//			}
//		}
//	}
//
//	// Static Objects
//	for (unsigned int i = 0; i < staticObjects.size(); i++)
//	{
//		auto staObject = staticObjects[i];
//		auto colliders = staObject->GetColliders();
//		auto meshColliders = staObject->GetMeshColliders();
//
//		// Normal Colliders
//		for (size_t j = 0; j < colliders.size(); j++)
//		{
//			if (Raycast(point, direction, colliders[j]) > 0)
//			{
//				glm::vec4 closestPoint = ClosestPointToCollider(point, colliders[j]);
//				float currDist = length2(closestPoint - point);
//
//				if (currDist < dist2)
//				{
//					dist2 = currDist;
//					object = staObject;
//				}
//			}
//		}
//
//		// Mesh Colliders
//		for (size_t j = 0; j < meshColliders.size(); j++)
//		{
//			if (Raycast(point, direction, &meshColliders[j]->GetBounds()) > 0)
//			{
//				glm::vec4 closestPoint = ClosestPointToCollider(point, &meshColliders[j]->GetBounds());
//				float currDist = length2(closestPoint - point);
//
//				if (currDist < dist2)
//				{
//					dist2 = currDist;
//					object = staObject;
//				}
//			}
//		}
//	}
//
//	return object;
//}


void ThirdPersonCameraComponent::DebugUpdateMouse(float deltaTime)
{
	glm::vec3 transVec(0, 0, 0);

	if (m_activeIM->IsKeyPressed(VK_UP))
	{
		transVec.z += m_MoveSpeed;
	}
	if (m_activeIM->IsKeyPressed(VK_DOWN))
	{
		transVec.z -= m_MoveSpeed;
	}
	if (m_activeIM->IsKeyPressed(VK_LEFT))
	{
		transVec.x -= m_MoveSpeed;
	}
	if (m_activeIM->IsKeyPressed(VK_RIGHT))
	{
		transVec.x += m_MoveSpeed;
	}
	if (m_activeIM->IsKeyPressed(VK_SHIFT))
	{
		/*DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		DirectX::XMMATRIX temp_camera = DirectX::XMLoadFloat4x4(&Camera);
		DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(translation, temp_camera);
		DirectX::XMStoreFloat4x4(&Camera, result);*/
		transVec.y -= m_MoveSpeed;

	}
	if (m_activeIM->IsKeyPressed('V'))
	{
		/*DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		DirectX::XMMATRIX temp_camera = DirectX::XMLoadFloat4x4(&Camera);
		DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(translation, temp_camera);
		DirectX::XMStoreFloat4x4(&Camera, result);*/
		transVec.y += m_MoveSpeed;
	}

	m_Camera = glm::translate(m_Camera, transVec*deltaTime);

	if (m_activeIM->IsKeyPressed(VK_RBUTTON))
	{
		float rotSpd = 0.1f;
		int dx, dy;
		m_activeIM->GetDeltaMousePos(dx, dy);

		glm::vec4 pos = glm::vec4(m_Camera[3][0], m_Camera[3][1], m_Camera[3][2], m_Camera[3][3]);

		m_Camera[3][0] = 0;
		m_Camera[3][1] = 0;
		m_Camera[3][2] = 0;



		//glm::mat4 rotX = glm::eulerAngleXY((float)dy * rotSpd * deltaTime, (float)dx * rotSpd * deltaTime);
		m_Camera = glm::rotate(m_Camera, dy * rotSpd * deltaTime, glm::vec3(1, 0, 0));
		m_Camera = glm::rotate(m_Camera, dx * rotSpd * deltaTime, glm::vec3(0, 1, 0));
		const glm::vec3 oop(0.0f, 1.0f, 0.0f);
		glm::vec3 z(m_Camera[2].x, m_Camera[2].y, m_Camera[2].z);
		glm::vec3 tx = glm::normalize(glm::cross(oop, z));
		glm::vec3 ny = glm::normalize(glm::cross(z, tx));
		glm::vec3 nx = glm::normalize(glm::cross(ny, z));
		m_Camera[0][0] = nx[0];
		m_Camera[0][1] = nx[1];
		m_Camera[0][2] = nx[2];

		m_Camera[1][0] = ny[0];
		m_Camera[1][1] = ny[1];
		m_Camera[1][2] = ny[2];

		m_Camera[3][0] = pos.x;
		m_Camera[3][1] = pos.y;
		m_Camera[3][2] = pos.z;
		//m_Camera = glm::lookAtLH(offset, glm::vec3(0, 0, 0), oop);
		//move the new camera pos.
		//glm::vec3 transl = glm::vec3(m_GameObject->m_Transform[3][0], m_GameObject->m_Transform[3][1], m_GameObject->m_Transform[3][2]);

		//m_Camera = glm::translate(m_Camera, transl);
	}
	m_render->SetCameraMatrix(m_Camera);

}

void ThirdPersonCameraComponent::UpdateMouse(float deltaTime)
{
	int height = m_render->GetHeight();
	int width = m_render->GetWidth();

	float rotSpd = 0.001f;
	float dx, dy;

	//Cant use this if we reset mouse pos
	//m_activeIM->GetDeltaMousePos(dx, dy);
	int x, y;
	m_activeIM->GetCurrentMousePos(x, y);
	dx = (float)(-(width / 2 - x));
	dy = (float)(-(height / 2 - y));


	//printf("%i %i \n", dx, dy);
	glm::tmat4x4<float, glm::aligned_highp> obj = glm::translate(glm::mat4(1), CameraOffsetPos);
	//printf("%f %f %f\n", CameraOffsetPos.x, CameraOffsetPos.y, CameraOffsetPos.z);
	const glm::vec3 oop(0.0f, 1.0f, 0.0f);

	rotY += dx*rotSpd/deltaTime;
	rotX += dy*rotSpd/deltaTime;
	rotX = glm::clamp(rotX, m_RotYFloor, m_RotYCap);
	obj = glm::yawPitchRoll(rotY, rotX, 0.0f) * obj;
	glm::vec3 offset = this->LookAtOffsetPos;

	//=================================
	glm::vec3 posToMoveAround = glm::vec3(m_target->GetWorldMatrix()[3][0], m_target->GetWorldMatrix()[3][1], m_target->GetWorldMatrix()[3][2]) + offset;
	m_Camera = obj;
	m_Camera[3][0] += posToMoveAround[0];
	m_Camera[3][1] += posToMoveAround[1];
	m_Camera[3][2] += posToMoveAround[2];
	//=================================

	glm::vec4 dir = m_Camera[3] - (m_target->m_Transform[3] + glm::vec4(LookAtOffsetPos, 0));
	float camLensqr = glm::length2(dir);
	glm::vec4 nDir = glm::normalize(dir);
	Object* val;// = CollisionDetection::RaycastAgainstClosest(m_GameObject->m_Transform[3] + glm::vec4(LookAtOffsetPos, 0), nDir, m_GameObject->m_ObjManager);
	glm::vec4 lookAtPos = m_target->m_Transform[3] + glm::vec4(LookAtOffsetPos, 0);
	glm::vec4 hit = lookAtPos + dir;
	val = m_target->m_ObjManager->m_StaticObjectQuadTree->Retrieve(lookAtPos, nDir, hit);
	if (val)
	{
		float ColLensqr = glm::length2(hit - lookAtPos);
		if (ColLensqr < camLensqr)
		{
			//Wall is closer. We need to clamp it.
			//printf("Tag : %i Col : %f Cam : %f \n", val->tag, ColLensqr, camLensqr);
			//offset *= (glm::dot(dir, (hit - lookAtPos)) - .1f);
			//=================================
			//printf("Obj : %f %f %f\n ", m_GameObject->m_Transform[3][0], m_GameObject->m_Transform[3][1], m_GameObject->m_Transform[3][2]);
			//printf("Camera : %f %f %f\n ", m_Camera[3][0], m_Camera[3][1], m_Camera[3][2]);
			//printf("Point : %f %f %f\n ", val->m_Transform[3][0], val->m_Transform[3][1], val->m_Transform[3][2]);
			hit = glm::lerp(hit, lookAtPos, 0.01f);
			m_Camera[3][0] = hit[0];
			m_Camera[3][1] = hit[1];
			m_Camera[3][2] = hit[2];
			//printf("Camera : %f %f %f\n ", m_Camera[3][0], m_Camera[3][1], m_Camera[3][2]);

			//=================================
		}
	}
	m_render->SetCameraMatrix(m_Camera);

	//this can be optimized.
	POINT pos;
	pos.x = width / 2;
	pos.y = height / 2;
	ClientToScreen(DKEngine::GetInstance()->getHwnd(), &pos);
	if (GetActiveWindow() != DKEngine::GetInstance()->getHwnd()) return;
	SetCursorPos(pos.x, pos.y);

	//After all that calc. We need to push the camera back in if it is behind a wall.
	//so from player pos we raycast to the cam. then go from there


}

void ThirdPersonCameraComponent::ChangeTarget(float deltaTime)
{
	m_timer += deltaTime;
	if (m_timer >= 1.0f)
	{
		m_players = m_GameObject->m_ObjManager->GetObjectsByTag(Object::Player);
	}

	if (m_activeIM->IsKeyDown(VK_LBUTTON))
	{
		m_SpecTargetIndx++;
		if (m_SpecTargetIndx >= m_players.size())
		{
			m_SpecTargetIndx = 0;
		}
		m_target = m_players[m_SpecTargetIndx];
	}
}


/*
void ThirdPersonCameraComponent::DebugUpdateMouse(float deltaTime, float x, float y)
{

const float lookscaler = 5.0f;
const float movescaler = 5.0f;
roty -= x * lookscaler;
rotx -= y * lookscaler;
if (rotx >= 3.14159f / 2.0f)
rotx = 3.14159f / 2.0f - 0.001f;
if (rotx <= -3.14159f / 2.0f)
rotx = -3.14159f / 2.0f + 0.001f;



if (GetAsyncKeyState('W'))
{
posx += cos(roty)*cos(rotx)*deltaTime*movescaler;
posz += sin(roty)*cos(rotx)*deltaTime*movescaler;
posy += sin(rotx)*deltaTime*movescaler;
}
if (GetAsyncKeyState('A'))
{
posz += cos(roty)*deltaTime*movescaler;
posx -= sin(roty)*deltaTime*movescaler;
}
if (GetAsyncKeyState('S'))
{
posx -= cos(roty)*cos(rotx)*deltaTime*movescaler;
posz -= sin(roty)*cos(rotx)*deltaTime*movescaler;
posy -= sin(rotx)*deltaTime*movescaler;
}
if (GetAsyncKeyState('D'))
{
posz -= cos(roty)*deltaTime*movescaler;
posx += sin(roty)*deltaTime*movescaler;
}


if (GetAsyncKeyState(VK_SPACE))
posy += deltaTime*movescaler;
if (GetAsyncKeyState(VK_LSHIFT))
posy -= deltaTime*movescaler;

glm::vec3 EyePosition = glm::vec3(posx, posy, posz);
glm::vec3 FocusPosition = glm::vec3(cos(roty)*cos(rotx) + posx, sin(rotx) + posy, sin(roty)*cos(rotx) + posz);
glm::vec3 UpDirection = glm::vec3(0.0f, 1.0f, 0.0f);



//glm::mat4 LookAt = glm::lookAtLH(EyePosition, FocusPosition, UpDirection);
//m_constantBufferData.view = glm::transpose(LookAt);
}
*/