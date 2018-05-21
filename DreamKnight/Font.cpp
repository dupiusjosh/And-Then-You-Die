#include "Font.h"
#include "Renderer.h"
#include "tinyxml2\tinyxml2.h"
#include "lodepng\lodepng.h"
#include "Utils.h"
#include <string.h>

Font::Font(const char* orig_dir)
{
	std::string path = DataManager::GetInstance()->GetAssetPath(orig_dir);

	char drive[60];
	char dir[60];
	_splitpath_s(path.c_str(), drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	if (doc->LoadFile(path.c_str()))
	{
		Utils::PrintError("failed to load Font %s\n", path.c_str());
		return;
	}
	printf("loading font from path: %s\n", path.c_str());
	tinyxml2::XMLElement* eleFont = doc->FirstChildElement();
	size = eleFont->IntAttribute("size");
	family = eleFont->Attribute("family");
	maxHeight = eleFont->IntAttribute("height");
	style = eleFont->Attribute("style");
	const char* relativepath = eleFont->Attribute("file");
	for (tinyxml2::XMLElement* eleChar = eleFont->FirstChildElement(); eleChar != nullptr; eleChar = eleChar->NextSiblingElement())
	{
		Char* ch = new Char();
		ch->height = maxHeight;
		ch->width = eleChar->IntAttribute("width");
		ch->referance = eleChar->Attribute("code")[0];

		sscanf_s(eleChar->Attribute("offset"), "%d %d", &ch->offsetX, &ch->offsetY);
		sscanf_s(eleChar->Attribute("rect"), "%d %d %d %d", &ch->rectX, &ch->rectY, &ch->rectWidth, &ch->rectHeight);

		for (tinyxml2::XMLElement* eleKerning = eleChar->FirstChildElement(); eleKerning != nullptr; eleKerning = eleKerning->NextSiblingElement())
		{
			char id = *eleKerning->Attribute("id");
			int advance = eleKerning->IntAttribute("advance");
			ch->kerning.insert(std::make_pair(id, advance));
		}

		characters.insert(std::make_pair(ch->referance, ch));
		//delete ch;
	}
	printf("\n");
	char pngpath[120];
	sprintf_s(pngpath, "%s%s%s", drive, dir, relativepath);
	delete doc;

	Renderer* renderer = Renderer::GetInstance();
	texture = DataManager::GetInstance()->LoadTexture(pngpath);
	if (texture == LOADERROR)
	{
		Utils::PrintError("Failed to load font texture (%s)\n", pngpath);
		width = 0;
		height = 0;
	}
	else
	{
		Texture* tex = renderer->GetTexture(texture);
		width = tex->width;
		height = tex->height;
	}
	buffer = renderer->CreateBuffer(sizeof(RenderableChar) * 64);
	shader = renderer->LoadShader("Font");
	//
	//std::vector<unsigned char> buff;
	//uint32_t width, height;
}

Font::~Font()
{
	for (auto& c : characters)
	{
		delete c.second;
		c.second = nullptr;
	}
}

int Font::GetKerning(Char* pc, Char* c)
{
	if (pc == nullptr || c == nullptr)
		return 0;
	int forward = pc->width;
	auto blah = pc->kerning.find(c->referance);
	if (blah != pc->kerning.end())
		forward += blah->second;
	return forward;
}

int Font::getWidthOfString(std::string str)
{
	int width = 0;
	Char* pc = nullptr;
	for (char c : str)
	{
		Char* ch = characters[c];
		width += GetKerning(pc, ch);
		pc = ch;
	}
	if(pc)
		width += pc->width;
	return width;
}

int Font::getHeightOfString(std::string str)
{
	return maxHeight;
}

/*
void Font::RenderText(std::string str, float x, float y)
{
	Renderer* renderer = Renderer::GetInstance();
	StringRenderable renderable;
	Char* prevChar = nullptr;
	float width = (float)renderer->GetWidth();
	float height = (float)renderer->GetHeight();
	float inv2Width = 2.0f / width;
	float inv2Height = 2.0f / height;
	float invFontWidth = 1.0f / this->width;
	float invFontHeight = 1.0f / this->height;
	float _x = -1 + ((float)x / (float)width) * 2;
	float _y = ((float)y / (float)height) * 2;
	for (size_t i = 0; i < str.length() && i < 64; ++i)
	{
		char c = str.c_str()[i];
		Char* ch = characters[c];
		_x += GetKerning(prevChar, ch) * inv2Width;
		RenderableChar rc;
		rc.width = (float)ch->rectWidth * inv2Width;
		rc.height = (float)ch->rectHeight * inv2Height;
		rc.offsetX = (float)ch->offsetX * inv2Width;
		rc.offsetY = (float)ch->offsetY * inv2Height;
		rc.rectX = (float)ch->rectX * invFontWidth;
		rc.rectY = (float)ch->rectY * invFontHeight;
		rc.rectWidth = (float)ch->rectWidth * invFontWidth;
		rc.rectHeight = (float)ch->rectHeight * invFontHeight;
		rc.x = _x;
		rc.y = _y;
		rc.z = maxHeight*inv2Height;
		rc.w = 1.0f;
		renderable.chars.push_back(rc);
		prevChar = ch;
	}
	RenderableChar rc;
	ZeroMemory(&rc, sizeof(rc));
	while (renderable.chars.size() < 64)
		renderable.chars.push_back(rc);

	Mesh* mesh = renderer->createPlane();
	switch (mesh->type)
	{
	case TRIANGLE_LIST:
		renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case LINE_LIST:
		renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	}

	renderer->UseShader(shader);

	//D3D11_MAPPED_SUBRESOURCE p;
	//ZeroMemory(&p, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//renderer->GetDeviceContext()->Map(buffer->buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &p);
	//memcpy(p.pData, renderable.chars.data(), sizeof(RenderableChar) * 64);
	//renderer->GetDeviceContext()->Unmap(buffer->buffer, 0);
	//renderer->GetDeviceContext()->IASetInputLayout(fontRendering->input_layout);
	//renderer->GetDeviceContext()->PSSetShader(fontRendering->pixel_shader, nullptr, 0);
	//renderer->GetDeviceContext()->VSSetShader(fontRendering->vertex_shader, nullptr, 0);
	renderer->GetDeviceContext()->UpdateSubresource(buffer->buffer, 0, NULL, renderable.chars.data(), 0, 0);
	renderer->GetDeviceContext()->VSSetConstantBuffers(0, 1, &buffer->buffer);

	renderer->UseTexture(texture);

	//deviceContext->PSSetShaderResources(1, 1, &font->textureSRV);
	//renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &texture->textureSRV);
	//renderer->GetDeviceContext()->PSSetSamplers(0, 1, &texture->pSampler);

	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;
	renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
	renderer->GetDeviceContext()->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	renderer->GetDeviceContext()->DrawIndexedInstanced(mesh->m_NumIndicies, renderable.chars.size(), 0, 0, 0);
}
*/
