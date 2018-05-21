#include "Canvas.h"
#include "InputManager.h"
#include "GuiElement.h"
#include "Renderer.h"

Canvas::~Canvas()
{
	for (GuiElement* e : m_Elements)
	{

		delete e;
	}
	m_Elements.clear();
}

void Canvas::AddElement(GuiElement * element)
{
	//element->canvas = this;
	m_Elements.push_back(element);
}

void Canvas::Update(float deltaTime)
{
	Object::Update(deltaTime);
	InputManager* im = InputManager::GetInstance();

	//int tx, ty;
	//im->GetCurrentMousePos(tx, ty);
	//printf("%d, %d\n", tx, ty);
	//Renderer::GetInstance()->AddLine(glm::vec4(0, tx, ty, 1), glm::vec4(0, tx + 1, ty + 1, 1));

	if (im->IsKeyDown(1))//Left Mouse Button
	{
		int tx, ty;
		im->GetCurrentMousePos(tx, ty);
		float x = (float)tx / Renderer::GetInstance()->GetRealWidth();
		float y = (float)ty / Renderer::GetInstance()->GetRealHeight();
		m_SelectedElement = CheckTarget(x, y);
		if (m_SelectedElement != nullptr)
			printf("%p element half-clicked!\n", m_SelectedElement);
	}

	if (im->IsKeyUp(1) && m_SelectedElement != nullptr)//Left Mouse Button
	{
		int tx, ty;
		im->GetCurrentMousePos(tx, ty);
		float x = (float)tx / Renderer::GetInstance()->GetRealWidth();
		float y = (float)ty / Renderer::GetInstance()->GetRealHeight();
		if (m_SelectedElement == CheckTarget(x, y))
		{
			if (m_FocusedElement)
				m_FocusedElement->UnFocus();
			m_SelectedElement->OnClicked();
			m_FocusedElement = m_SelectedElement;
			printf("%p element clicked!\n", m_SelectedElement);
		}
	}
	for (GuiElement* elem : m_Elements)
		elem->Update();
}

//GuiElement * Canvas::CheckTarget(float x, float y)
//{
//	for (GuiElement* elem : m_Elements)
//	{
//		auto renderer = Renderer::GetInstance();
//		glm::vec2 off = elem->GetOffset();
//		glm::vec2 size = elem->GetSize();
//		size.x /= renderer->GetRealWidth();
//		size.y /= renderer->GetRealHeight();
//		if (x >= off.x && y >= off.y)
//		{
//			off += size;
//			if (x <= off.x && y <= off.y)
//			{
//				return elem;
//			}
//		}
//	}
//	return nullptr;
//}
GuiElement * Canvas::CheckTarget(float x, float y)
{
	GuiElement* ret = nullptr;
	for (GuiElement* elem : m_Elements)
	{
		auto renderer = Renderer::GetInstance();
		glm::vec2 off = elem->GetOffset();
		glm::vec2 size = elem->GetSize();
		size.x /= renderer->GetRealWidth();
		size.y /= renderer->GetRealHeight();
		if (x >= off.x && y >= off.y)
		{
			off += size;
			if (x <= off.x && y <= off.y)
			{
				ret = elem;
			}
		}
	}
	return ret;
}