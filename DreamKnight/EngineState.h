#pragma once
#include <mutex>
#include <thread>
class ObjectManager;
class Renderer;

class EngineState
{
protected:
	bool m_Loaded = false;
	std::mutex loadLock;
	std::thread* loadThread;

	ObjectManager * m_ObjManager;
	virtual void Load() {};
	void FINISH()
	{
		this->loadLock.lock();
		m_Loaded = true;
		this->loadLock.unlock();
	}
public:
	

	EngineState(ObjectManager * objMan) : m_ObjManager(objMan)
	{
		//Noffin.
	};
	EngineState() {};
	virtual ~EngineState()
	{
		loadThread->join();
		delete loadThread;
	};
	virtual void StartLoad()
	{
		loadThread = new std::thread(&EngineState::Load, this);
	}
	//May remove this....
	virtual bool RunMainLoop()
	{
		return isLoaded();
	}
	//This function execs every frame, after the state has been FINISHED.
	virtual void Update(ObjectManager* om, Renderer * rn, float deltaTime)
	{

	}
	bool isLoaded()
	{
		bool res;
		loadLock.lock();
		res = m_Loaded;
		loadLock.unlock();
		return res;
	};
};

