#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <vector>
#ifdef WIN32
	#include <d3d11.h>
#else
	#include <d3d11.h>
	#undef WIN32
#endif
#include <unordered_map>
#include "glm.h"
#include "Object.h"
#include "DataManager.h"
#include "Mesh.h"
#include "ObjectManager.h"
#include "LightComponent.h"
#include "Frustum.h"
#include <mutex>

class GuiElement;
class GuiImage;
class GuiText;
class GuiTextbox;

#define USE_DIRECTX_RENDERER
#pragma comment (lib, "d3d11.lib")

#define MAX_LIGHTS 32

struct Font;

struct pipeline_state_t
{
	ID3D11Texture2D *render_target_texture = nullptr;
	ID3D11RenderTargetView *render_target = nullptr;
	ID3D11Texture2D *depthStencilBuffer = nullptr;
	ID3D11DepthStencilState *depthStencilState = nullptr;
	ID3D11DepthStencilView *depthStencilView = nullptr;
	ID3D11RasterizerState *rasterState = nullptr;
	ID3D11RasterizerState *debugRasterState = nullptr;
};

struct PerObjectBatchData
{
	glm::mat4 m_Model;
	glm::vec4 m_Color;
};

struct ModelViewProjectionConstantBuffer
{
	PerObjectBatchData data[32];
	glm::mat4 view;
	glm::mat4 projection;
};

enum RenderBufferType
{
	BUFFER_NONE = 0,
	BUFFER_COLOR = 1,
	BUFFER_DEPTH = 2,
	BUFFER_STENCIL = 4,
};

struct ShaderProgram
{
	ID3D11InputLayout* m_InputLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	void Release()
	{
		if (m_InputLayout) m_InputLayout->Release(), m_InputLayout = nullptr;
		if (m_VertexShader) m_VertexShader->Release(), m_VertexShader = nullptr;
		if (m_PixelShader) m_PixelShader->Release(), m_PixelShader = nullptr;

		delete this;
	};
};

struct Texture
{
	ID3D11Texture2D* pTexture = nullptr;
	ID3D11ShaderResourceView* textureSRV = nullptr;
	ID3D11SamplerState* pSampler = nullptr;
	uint32_t width=-1, height=-1;
	size_t m_Id = -1;

	void Release()
	{
		if (pTexture) pTexture->Release(), pTexture = nullptr;
		if (textureSRV) textureSRV->Release(), textureSRV = nullptr;
		if (pSampler) pSampler->Release(), pSampler = nullptr;

		delete this;
	}
};

struct Buffer
{
	ID3D11Buffer* buffer;
	size_t size = 0;

	void UseBufferVS(size_t slot);
	void UseBufferPS(size_t slot);
	void WriteData(void* data);
	void WriteData(uint32_t size, void* data);
	void ClearAndWriteData(uint32_t size, void * data);

	void Release()
	{
		if (buffer) buffer->Release(), buffer = nullptr;
		delete this;
	}
};

struct Light
{
	glm::vec4 m_Position; //xyz = position, w = type
	glm::vec4 m_Direction;
	glm::vec4 m_Color;
};

class Renderer
{
private://Variables
	HWND m_Window = nullptr;

	uint32_t width, height;

	bool m_wireframe = false;
	bool m_renderBones = true;

	//DataManager* m_dm;

	pipeline_state_t* Pipeline = nullptr;
	//sub_pipeline_state_t* basicRendering = nullptr;
	//sub_pipeline_state_t* fontRendering = nullptr;

#ifdef _DEBUG
	ID3D11Debug * debug = nullptr;
#endif
	ID3D11Device * device = nullptr;
	ID3D11DeviceContext * deviceContext = nullptr;
	IDXGISwapChain * swapchain = nullptr;
	//ID3D11RenderTargetView *renderTargetView = nullptr;
	
	int defaultShader, animamationShader, texturedShader, guiShader;

	ID3D11Buffer *m_constantBuffer = nullptr;
	ModelViewProjectionConstantBuffer	m_constantBufferData;

	ID3D11DepthStencilState *depthStencilState2D = nullptr;
	ID3D11DepthStencilState *depthStencilState3D = nullptr;

	glm::mat4 boneMatrices[64];
	ID3D11Buffer* AnimationFrame = nullptr;
	ID3D11Buffer* InvBindPos = nullptr;


	ID3D11Buffer *m_fontBuffer = nullptr;
	ID3D11Buffer *m_guiBuffer = nullptr;
	Buffer *m_LightBuffer = nullptr;


	ID3D11BlendState* blendState;
/*
#pragma region DebugData
	CircleBuffer<vec4> m_DebugLines = CircleBuffer<vec4>(128);
	ID3D11Buffer *m_DebugVertexBuffer = nullptr;
	ID3D11Buffer *m_DebugIndexBuffer = nullptr;
#pragma endregion
*/
	size_t ErrorTexture;

	Mesh* CharMesh = nullptr;
	std::unordered_map<std::string, Mesh*> renderables;
	std::vector<Mesh*> external_renderables;
	//std::vector<GuiElement*> m_GuiElements;

	std::unordered_map<std::string, int> shadersFromName;

	std::vector<ShaderProgram*> shaders;
	std::vector<Texture*> textures;
	std::vector<Buffer*> buffers;
	std::vector<Light> lights;


	bool dirtyLines = false;
	std::vector<Mesh::Vertex> lines;
	ID3D11Buffer *m_LineBuffer = nullptr;

	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;
	D3D11_VIEWPORT viewport;

	std::unordered_map<Mesh*, std::vector<PerObjectBatchData>*> m_Batcher;

	Frustum m_Frustum;

	std::mutex AsyncLoading;

private://Functions
	IDXGIAdapter1* GetBestGpu();

	void SetupLights();

	void UpdateLights();



public:
	static Renderer*& GetInstance()
	{
		static Renderer* m_Instance;
		return m_Instance;
	}
	Renderer(HWND window, int width, int height);
	~Renderer();
	//void DebugDrawLine(vec4 start, vec4 end);
	
	//void loadFontShaders();
	//void SetupDebugShit();
	//void loadFontShaders();
	int LoadShader(char* name);
	//size_t LoadTexturePNG(char * name, uint8_t flags = 0);
	//size_t LoadTextureFromMemory(char* name, const uint8_t * file_data, size_t len, uint8_t flags);
	//size_t LoadTexture(char * name, uint8_t flags = 0);
	size_t CreateTexture(const char * name, uint8_t * data, int len, int width, int height);
	Texture* GetTexture(size_t id);
	Buffer* CreateBuffer(size_t size);
	//Functions

	//void setDataManger(DataManager* _dm);
	void UseShader(int i);

	void UseTexture(size_t i);

	void RenderText(Font * font, std::string str, float x, float y);

	void RenderImage(size_t textureId, float x, float y, float sx = 1.0f, float sy = 1.0f);

	void RenderImageResource(uint32_t resourceId, float x, float y, float sx = 1.0f, float sy = 1.0f);

	void RenderGuiElements();

	void RenderGuiElement(GuiElement * elem);

	void RenderGuiImage(GuiImage* elem);

	void RenderGuiText(GuiText * elem);

	void RenderGuiTextbox(GuiTextbox * elem);

	void AddLight(Light light);
	//void AddGuiElement(GuiElement* element);

	Mesh* createPlane();
	void RenderMesh(Mesh * renderable, glm::mat4 transform);
	//void UpdateWindowSize();

	void UpdateMouse(float deltaTime, float x, float y);

	void DebugRender(ObjectManager * objManager);

	//void RenderDebug();
	void RenderScene(ObjectManager* objManager);
	void AddLine(glm::vec4 start, glm::vec4 end, glm::vec4 color = glm::vec4(1));
	void AddLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1));
	void ClearLines();
	void RenderDebugData();
	void RenderObject(Object* obj);
	void BatchObject(Object * obj);
	void BatchObjects(std::list<Object*>::iterator begin, std::list<Object*>::iterator end);
	void RenderBatch();
	
	void Present();

	void SetSize(int width, int height);

	void Clear(uint32_t buffer);

	Mesh * CreateRenderable(Mesh * mesh);

	uint32_t GetWidth();
	uint32_t GetHeight();
	uint32_t GetRealWidth();
	uint32_t GetRealHeight();
	float GetInvWidth();
	float GetInvHeight();
	ID3D11Device* GetDevice();

	ID3D11DeviceContext * GetDeviceContext();

	//Mesh * CreateRenderable(char * name, Mesh * mesh);
	//void SetTexture(Font* font);

	//void RenderText(Font*, std::string, float x, float y);
	
	//void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	void SetCameraMatrix(glm::mat4);
	glm::mat4 GetCameraMatrix();
};