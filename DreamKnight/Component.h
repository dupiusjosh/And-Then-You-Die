#pragma once
class Object;

struct Component
{
private:
	bool m_Enabled=true;

public:
	virtual void SetEnabled(bool b) { m_Enabled = b; }
	virtual bool IsEnabled() { return m_Enabled; }
	virtual ~Component();
	Object * m_GameObject=nullptr;

	virtual void OnUpdate(float deltaTime) = 0;
	void AfterUpdate(float deltaTime) {};
	virtual void OnStart() {};
};