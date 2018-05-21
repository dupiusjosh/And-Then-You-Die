#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Player.h"
#include "glm.h"

Player::Player(Object* _obj)
{
	m_GameObject = _obj;
}

Player::~Player()
{

}

void Player::OnUpdate(float _dTime) 
{
	//glm::mat4 tempTransform = m_GameObject->m_Transform;
	//for (unsigned int i = 0; i < m_Input->GetThisFrame().size(); i++)
	//{
	//	if (m_Input->GetThisFrame()[i] == UP)
	//	{
	//		//printf("UP");
	//		glm::vec3 trans = { 0, 0, _dTime * 75 };
	//		m_GameObject->m_Transform = glm::translate(m_GameObject->m_Transform, trans);
	//	}
	//	else if (m_Input->GetThisFrame()[i] == DOWN)
	//	{
	//		//printf("DOWN");
	//		glm::vec3 trans = { 0, 0, -_dTime * 75 };
	//		m_GameObject->m_Transform = glm::translate(m_GameObject->m_Transform, trans);
	//	}

	//	if (m_Input->GetThisFrame()[i] == LEFT)
	//	{
	//		//printf("LEFT");
	//		glm::vec3 trans = { -_dTime * 75, 0, 0 };
	//		m_GameObject->m_Transform = glm::translate(m_GameObject->m_Transform, trans);
	//	}
	//	else if (m_Input->GetThisFrame()[i] == RIGHT)
	//	{
	//		//printf("RIGHT");
	//		glm::vec3 trans = { _dTime * 75, 0, 0 };
	//		m_GameObject->m_Transform = glm::translate(m_GameObject->m_Transform, trans);
	//	}
	//}
	
	/*for (unsigned int i = 0; i < m_Input->GetThisFrame().size();i++)
	{
		switch (m_Input->GetThisFrame()[i])
		{
		case UP:
			printf("player UP\n");
			break;
		case LEFT:
			printf("player LEFT\n");
			break;
		case DOWN:
			printf("player DOWN\n");
			break;
		case RIGHT:
			printf("player RIGHT\n");
			break;
		default:
			printf("BORK");
			break;
		}
	}
	if (m_Input->GetTemp() != -1)
	{
		switch (m_Input->GetTemp())
		{
		case UP:
			printf("player UP\n");
			break;
		case LEFT:
			printf("player LEFT\n");
			break;
		case DOWN:
			printf("player DOWN\n");
			break;
		case RIGHT:
			printf("player RIGHT\n");
			break;
		default:
			printf("BORK");
			break;
		}
	}*/
}