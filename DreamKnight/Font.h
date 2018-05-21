#pragma once
#include <unordered_map>
#include <string>
#include <d3d11.h>
#include <stdint.h>
#include "Renderer.h"

struct RenderableChar
{
	float x, y, z, w;
	float width, height, offsetX, offsetY;
	float rectX, rectY, rectWidth, rectHeight;
};

struct StringRenderable
{
	std::vector<RenderableChar> chars;
};

struct Char
{
	char referance;
	int width, height;
	int offsetX, offsetY;
	int rectX, rectY, rectWidth, rectHeight;
	std::unordered_map<char, int> kerning;
};

struct Font
{
	static RenderableChar m_fontBufferData[64];
	size_t texture;
	Buffer* buffer;
	int shader;
	uint32_t width, height, maxHeight;
	int size;
	std::string family;
	std::string style;
	std::unordered_map<char, Char*> characters;
	
public:
	Font(const char* path);
	~Font();

	int GetKerning(Char * pc, Char * c);
	int getWidthOfString(std::string str);
	int getHeightOfString(std::string str);
	//void RenderText(std::string str, float x, float y);
};