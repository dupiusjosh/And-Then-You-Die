#include "GuiTextbox.h"
#include "InputManager.h"
#include "Utils.h"

GuiTextbox::~GuiTextbox()
{
	delete m_Text;
}

char* GuiTextbox::GetText()
{
	return m_Text;
}

Font* GuiTextbox::GetFont()
{
	return m_Font;
}

void GuiTextbox::SetText(char * data)
{
	strncpy_s(m_Text, 512, data, 511);
}

void GuiTextbox::SetFont(Font * font)
{
	m_Font = font;
}

void GuiTextbox::SetImage(size_t id)
{
	m_ImageID = id;
}

size_t GuiTextbox::GetImage()
{
	return m_ImageID;
}

glm::vec2 GuiTextbox::GetSize()
{
	glm::vec2 textSize = glm::vec2(m_Font->getWidthOfString(m_Text), m_Font->getHeightOfString(m_Text));
	Texture* tex = Renderer::GetInstance()->GetTexture(m_ImageID);
	glm::vec2 imageSize = glm::vec2(tex->width, tex->height);
	return glm::vec2(max(textSize.x, imageSize.x), max(textSize.y, imageSize.y));
}

glm::vec2 GuiTextbox::GetTextOffset()
{
	glm::vec2 local_offset = glm::vec2(m_Font->getWidthOfString(m_Text), m_Font->getHeightOfString(m_Text));
	local_offset.x *= 0.5f;
	local_offset.y *= 0.5f;
	Renderer* renderer = Renderer::GetInstance();
	local_offset /= glm::vec2(renderer->GetWidth(), renderer->GetHeight());
	return m_Offset - local_offset;
}

void GuiTextbox::OnClicked()
{
	focused = true;
}

void GuiTextbox::UnFocus()
{
	focused = false;
}

void GuiTextbox::Backspace()
{
	if (m_Pos > 0)
	{
		m_Text[--m_Pos] = 0;
	}
}

void GuiTextbox::AddChar(char c)
{
	if (m_Pos < 512)
	{
		m_Text[m_Pos] = c;
		m_Text[m_Pos + 1] = 0;
		m_Pos++;
	}

	int imgWidth = Renderer::GetInstance()->GetTexture(m_ImageID)->width;
	while (m_Font->getWidthOfString(m_Text) > imgWidth - 5)
		Backspace();
}

void GuiTextbox::Update()
{
	if (!focused)
		return;

	InputManager* im = InputManager::GetInstance();

	auto input = InputManager::GetInstance();

	for (unsigned char i = 0; i < 255; ++i)
	{
		unsigned char pressed = input->IsKeyDown(i);
		if (pressed || input->GetKeyHoldTime(i)>0.5f)
		{
			if (pressed || rep != floorf(input->GetKeyHoldTime(i) * 15))
			{
				rep = floorf(input->GetKeyHoldTime(i) * 15);

				unsigned short data[3] = {};
				if (Utils::vk2ascii(i, data))
				{
					if (data[0] == '\b')
						Backspace();
					else if (data[0] < 32)
						continue;
					else
						AddChar((char)data[0]);
				}
			}
		}
	}

	if (input->IsKeyPressed(VK_CONTROL) && input->IsKeyDown('V'))
	{
		char* clip;

		if (OpenClipboard(NULL)) {
			clip = (char*)GetClipboardData(CF_TEXT);
			CloseClipboard();
		}

		for (size_t i = 0; i < strlen(clip) && m_Pos < 511; i++)
			AddChar(clip[i]);

		printf("%s\n", (char*)clip);
	}
}