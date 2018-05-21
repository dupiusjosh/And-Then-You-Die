#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "NetClient.h"
#include "NetServer.h"
#include "Renderer.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "AudioManager.h"
#include <windowsx.h>
#include "LevelGeneration\LevelGenerator.h"
#include "NavMesh.h"
class EngineState;
struct Font;
class DKEngine
{

	//GW::GReturn retval;
	//GW::SYSTEM::GInput* gInput;

	
	HWND hWnd;
	Renderer * m_Renderer = nullptr;
	AudioManager* m_AudioManager = nullptr;
	DataManager* m_DataManager;
	float width, height;
	bool m_Headless;
	bool m_Running;
	Font* m_Font;

	EngineState * loadingState = nullptr;
	EngineState * tempState=nullptr;



	void Init();
	void HandleMessages();
public:
	LevelGenerator * levelgen = nullptr;
	Navigation::NavMesh * navmesh = nullptr;
	NetServer *m_Serv = nullptr;
	NetClient *m_Client = nullptr;
	InputManager* m_inManager;
	ObjectManager* m_ObjManager = nullptr;
	static DKEngine *& GetInstance()
	{
		static DKEngine * instance;
		return instance;
	}

	HWND getHwnd() { return hWnd; }
	DKEngine(HWND h, bool headless = false);
	~DKEngine();
	int Run();
	void WindowSize(int, int);
	bool ChangeState(EngineState* state);
	Font* GetFont()
	{
		return m_Font;
	}
};