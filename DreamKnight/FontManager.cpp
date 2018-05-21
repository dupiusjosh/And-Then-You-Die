#include "FontManager.h"

FontManager * FontManager::GetInstance()
{
	static FontManager* m_FontManager = new FontManager();
	return m_FontManager;
}

Font * FontManager::GetFont(std::string name)
{
	auto iter = m_Fonts.find(name);
	if (iter == m_Fonts.end())
	{
		Font* f = new Font(name.c_str());
		m_Fonts.insert(make_pair(name, f));
		return f;
	}
	return iter->second;
}

void FontManager::CleanUp()
{
	for (auto iter : m_Fonts)
		delete iter.second;
}
