#pragma once
#include "Font.h"
#include <unordered_map>
#include <string>

class FontManager
{
	std::unordered_map<std::string, Font*> m_Fonts;
public:

	static FontManager* GetInstance();

	Font* GetFont(std::string name);
	void CleanUp();
};