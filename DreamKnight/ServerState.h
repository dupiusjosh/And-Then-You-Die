#pragma once
class NetServer;
class ServerState
{
protected:
	NetServer * m_Server;

public:
	ServerState() {};
	virtual void Start(NetServer *)=0;
	virtual void Update(float deltaTime) = 0;

};