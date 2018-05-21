
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "DKEngine.h"
#include "glm.h"
#include "MeshComponent.h"
#include "SoundComponent.h"
#include "Player.h"
#include "PickupManager.h"
#include "ItemRef.h"

//#include <windowsx.h>


#include "AABBCollider.h"
#include "CapsuleCollider.h"
#include "SphereCollider.h"
#include "RigidBodyComponent.h"
#include "BSPTree.h"

#include <queue>
#include "CollisionDetection.h"
#include "Font.h"
#include "AnimationComponent.h"
#include "MaterialComponent.h"
#include "Utils.h"
#include "Pickup.h"

#include "QuadTree.h"
#include "EngineState.h"
#include "MenuState.h"
#include "FontManager.h"
#define NETWORK_STUFF 1

void DKEngine::Init()
{
	m_inManager = InputManager::GetInstance();//new InputManager();

	RECT rect;
	GetClientRect(hWnd, &rect);
	width = (float)(rect.right - rect.left);
	height = (float)(rect.bottom - rect.top);

	//printf("Loading Text\n");
	m_Font = FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml");
	//printf("....Done\n");

	ChangeState(new MenuState(m_ObjManager));
}

void DKEngine::HandleMessages()
{
	MSG msg;

	while (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);

		if (GetActiveWindow() != hWnd)
		{
			if (msg.message != WM_QUIT)
				continue;
		}
		switch (msg.message)
		{
			case WM_QUIT:
			{
				m_Running = false;
			} break;

			case WM_KEYDOWN:
			{
				m_inManager->SetKeyPressed((uint8_t)msg.wParam);
			} break;
			case WM_KEYUP:
			{
				m_inManager->SetKeyUp((uint8_t)msg.wParam);
			} break;
			case WM_LBUTTONDOWN:
			{
				m_inManager->SetKeyPressed(VK_LBUTTON);
			}break;
			case WM_LBUTTONUP:
			{
				m_inManager->SetKeyUp(VK_LBUTTON);
			}break;
			case WM_RBUTTONDOWN:
			{
				m_inManager->SetKeyPressed(VK_RBUTTON);
			}break;
			case WM_RBUTTONUP:
			{
				m_inManager->SetKeyUp(VK_RBUTTON);

			}break;
			case WM_MBUTTONDOWN:
			{
				m_inManager->SetKeyPressed(VK_MBUTTON);
			}break;
			case WM_MBUTTONUP:
			{
				m_inManager->SetKeyUp(VK_MBUTTON);

				//printf("mouse : %d\n", msg.wParam);
			}break;
			case WM_MOUSEMOVE:
			{
				//printf("mouse pos x : %3f\t", (GET_X_LPARAM(msg.lParam) / width) - 0.5f);
				//printf("mouse pos y : %3f\n", (GET_Y_LPARAM(msg.lParam) / height) - 0.5f);
				//printf("MOUSE MOVE: %i %i", GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
				m_inManager->SetCurrentMousePos(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
			}break;
		}

#if NETWORK_STUFF
		if (m_Client)
		{
			switch (msg.message)
			{
				case WM_QUIT:
				{
					m_Running = false;
				} break;

				case WM_KEYDOWN:
				{
					m_Client->UpdateKeyValue((uint8_t)msg.wParam, true);
				} break;
				case WM_KEYUP:
				{
					m_Client->UpdateKeyValue((uint8_t)msg.wParam, false);
				} break;
				case WM_LBUTTONDOWN:
				{
					m_Client->UpdateKeyValue((uint8_t)msg.wParam, true);
				}break;
				case WM_LBUTTONUP:
				{
					m_Client->UpdateKeyValue(VK_LBUTTON, false);
				}break;
				case WM_RBUTTONDOWN:
				{
					m_Client->UpdateKeyValue(VK_RBUTTON, true);
				}break;
				case WM_RBUTTONUP:
				{
					m_Client->UpdateKeyValue(VK_RBUTTON, false);
				}break;
				case WM_MBUTTONDOWN:
				{
					m_Client->UpdateKeyValue(VK_MBUTTON, true);
				}break;
				case WM_MBUTTONUP:
				{
					m_Client->UpdateKeyValue(VK_MBUTTON, false);
				}break;
				case WM_MOUSEMOVE:
				{
					//m_Client->SetCurrentMousePos(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
				}break;
			}
		}
#endif
	}
}

DKEngine::DKEngine(HWND h, bool headless)
{
	GetInstance() = this;
	hWnd = h;

	m_Headless = headless;
	if (!headless)
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		width = 1920;// (float)(rect.right - rect.left);
		height = 1080;// (float)(rect.bottom - rect.top);
		m_Renderer = new Renderer(hWnd, (int)width, (int)height);
		m_DataManager = DataManager::GetInstance();
		ItemRef::Init();
		//m_Renderer->setDataManger(m_DataManager);
	}
	else
	{
		m_DataManager = nullptr;
	}
	m_ObjManager = ObjectManager::GetInstance();
	
	navmesh = new Navigation::NavMesh();
	m_ObjManager->SetNavMesh(navmesh);
	m_AudioManager = AudioManager::GetInstance();

	Init();
	//printf("DKDONE\n");
}

DKEngine::~DKEngine()
{
	delete navmesh;
	delete m_ObjManager;
	delete tempState;
	delete loadingState;
	delete levelgen;
	delete m_inManager;
	delete m_Renderer;
	delete m_Client;
	delete m_Serv;
	delete m_AudioManager;
	delete m_DataManager;
}

int DKEngine::Run()
{
	//inManager = inManager->GetInstance();

	std::chrono::time_point<std::chrono::system_clock> lastnow;
	std::chrono::time_point<std::chrono::system_clock> rendStartTime;

	lastnow = std::chrono::system_clock::now();
	float time = 0, deltaTime = 0;
	float fpsUpdate = 0, fps = 0;
	int counter = 0;
	float time2 = 0;
	bool firstClick = true;

	m_Running = true;
	float rendPrct=0;
	float rendTime = 0;
	while (m_Running)
	{
		deltaTime = (float)((std::chrono::system_clock::now() - lastnow).count()) / 1e7f;
		lastnow = std::chrono::system_clock::now();
		//rendPrct = rendTime / deltaTime;
		if (tempState && (!loadingState || loadingState->isLoaded()))
		{
			m_ObjManager->ClearObjects(true);
			m_ObjManager->Update(0);
			delete loadingState;
			loadingState = tempState;
			loadingState->StartLoad();
			tempState = NULL;
		}
		if (loadingState)
		{
			loadingState->Update(m_ObjManager, m_Renderer, deltaTime);
			//returns true if I should execute the normal game loop
			if (loadingState->RunMainLoop())
			{
				
				m_inManager->Update(deltaTime);
				time += deltaTime;

				if (fpsUpdate < time)
				{
					fpsUpdate = time + 1;
					//printf("FPS UPDATE! {%d}\n", counter);
					fps = (float)counter;
					//printf("Render percent: %f %f %f\n", rendPrct, deltaTime, rendTime);
					counter = 0;
				}
				counter++;

				HandleMessages();

#if NETWORK_STUFF
				if (m_Client)
					m_Client->Update();
#endif
				m_ObjManager->Update(deltaTime);


				if (!m_Headless)
				{
					//if (m_inManager->IsKeyPressed(VK_F2))
					//{
					//Object* obj = m_ObjManager->GetObjectByTag(Object::Player);
					//if (obj)
					//{
					//	glm::vec4 pos = obj->m_Transform[3];
					//	//printf("pos: %f %f %f\n", pos.x, pos.y, pos.z);
					//	Light light;
					//	light.m_Position = pos;
					//	light.m_Position.w = (float)LightType::Point;
					//	light.m_Direction.w = 10.0f;
					//	light.m_Color = glm::vec4(1, 1, 1, 1);
					//	m_Renderer->AddLight(light);
					//}
					//}
					//rendStartTime = std::chrono::system_clock::now();
					//m_Renderer->UseTexture(m_Font);
					m_Renderer->RenderScene(m_ObjManager);
					m_Renderer->RenderGuiElements();
					char buff[60];
					sprintf_s(buff, "%0.0ffps", fps);
					m_Renderer->RenderText(m_Font, buff, 0, 0);

					//glm::vec4 hit;
					//if (m_ObjManager->m_StaticObjectQuadTree)
					//{
					//	m_ObjManager->m_StaticObjectQuadTree->Retrieve(glm::vec4(0, 1, 0, 1), glm::vec4(1, 0, 1, 0), hit);
					//	Renderer::GetInstance()->AddLine(glm::vec4(0, 1, 0, 1), hit);
					//}

					//if(m_ObjManager->m_StaticObjectQuadTree)
					//	m_ObjManager->m_StaticObjectQuadTree->DrawLines();

					//AABBCollider aabb;
					//aabb.SetMin(glm::vec4(-1, -1, -1, 1));
					//aabb.SetMax(glm::vec4(1, 1, 1, 1));
					//printf("%d\n", CollisionDetection::RaycastToAABB(glm::vec4(0, 2, 0, 1), glm::vec4(0, -1, 0, 0), &aabb));

					m_Renderer->RenderDebugData();
					m_Renderer->Present();
					//rendTime = (float)((std::chrono::system_clock::now() - rendStartTime).count()) / 1e7f;


					//moved for now
					m_Renderer->Clear(BUFFER_COLOR | BUFFER_DEPTH | BUFFER_STENCIL);
					m_Renderer->ClearLines();
				}
			}
		}
	}

	//delete font;
	return 0;
}

void DKEngine::WindowSize(int _width, int _height)
{
	if (m_Renderer)
		m_Renderer->SetSize(_width, _height);
	width = (float)_width;
	height = (float)_height;
}

//Pass in a new state, function will delete state if one is already loading.
bool DKEngine::ChangeState(EngineState * state)
{
	//if (!loadingState)
	//{
	//	//if null
	//	loadingState = state;
	//	loadingState->StartLoad();
	//	return true;
	//}
	//if (loadingState->isLoaded())
	//{
	//	delete loadingState;
	//	m_ObjManager->ClearObjects();
	//	loadingState = state;
	//	loadingState->StartLoad();
	//	return true;
	//}
	//delete state;
	//return false;
	tempState = state;
	return true;
}
