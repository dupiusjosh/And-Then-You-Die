#include "Renderer.h"
#include "glm.h"

#include <fstream>
#include <set>
#include <string>
#include "DataManager.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "lodepng/lodepng.h"
#include "tinyxml2\tinyxml2.h"
#include <AL\al.h>
#include <atlcomcli.h>
#include <wrl.h>
#include <thread>
#include <mutex>
#include <time.h>
#include "Utils.h"
#include "Font.h"
#include "AnimationComponent.h"
#include "MaterialComponent.h"
#include "MeshCollider.h"
#include "AudioManager.h"

#include "GuiImage.h"
#include "GuiText.h"
#include "GuiTextbox.h"
#include "Canvas.h"
#include "Frustum.h"

#define GRAPHICS_DEBUGGING 0
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "dxguid.lib")
#include <dxgi.h>

#define ReleaseIfExists(x) if(x) x->Release(), x = nullptr

#define BATCH_SIZE 32
#define LINE_LIMIT 16384

//using namespace glm;

template<typename t>
void SetDebugName(t* child, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	char dest[256], dest2[256];
	if (child != nullptr)
	{
		uint32_t size = 256;
		child->GetPrivateData(WKPDID_D3DDebugObjectName, &size, dest);
		dest[size] = '\n';
		dest[size + 1] = 0;
		if (size > 0)
			Utils::PrintWithColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY, dest);
		else
		{
			vsprintf_s(dest2, format, ap);
			sprintf_s(dest, "[%p] %s", child, dest2);
			if (child != nullptr)
			{
				child->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(dest), dest);
				Utils::PrintWithColor(FOREGROUND_GREEN, "Naming Directx object: (%s)\n", dest);
			}
		}
	}
	va_end(ap);
}

IDXGIAdapter1* Renderer::GetBestGpu()
{
	IDXGIAdapter1* pAdapter;
	std::vector <IDXGIAdapter1*> vAdapters;
	IDXGIFactory1* pFactory = NULL;

	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
		return nullptr;

	for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		vAdapters.push_back(pAdapter);

	if (pFactory)
		pFactory->Release();

	for (auto pAdapter : vAdapters)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		pAdapter->GetDesc(&adapterDesc);
		wprintf(L"%s: %d, %d\n", adapterDesc.Description, adapterDesc.DeviceId, adapterDesc.VendorId);
		//if(adapterDesc.DeviceId == )
	}

	uint32_t ids[] = { 4318, 4098, 32902, 5140 };

	for (auto id : ids)
	{
		for (auto pAdapter : vAdapters)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			pAdapter->GetDesc(&adapterDesc);
			if (id == adapterDesc.VendorId)
			{
				wprintf(L"using: %s\n", adapterDesc.Description);
				return pAdapter;
			}
		}
	}

	assert(vAdapters.size() > 0);
	return vAdapters[0];
}

void Renderer::SetupLights()
{
	lights.resize(MAX_LIGHTS);
	m_LightBuffer = CreateBuffer(sizeof(Light) * 32);
}

void Renderer::UpdateLights()
{
	m_LightBuffer->ClearAndWriteData(lights.size() * sizeof(Light), lights.data());
	lights.clear();
}

void Renderer::AddLight(Light light)
{
	if(lights.size() < MAX_LIGHTS)
		lights.push_back(light);
}

Renderer::Renderer(HWND hWnd, int width, int height)
{
	GetInstance() = this;
	m_Window = hWnd;
	this->width = width;
	this->height = height;

	UINT flags = 0;
#ifndef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureArray[] = {
		//D3D_FEATURE_LEVEL_12_0,
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		//D3D_FEATURE_LEVEL_10_1,
		//D3D_FEATURE_LEVEL_10_0,
	};

	UINT featureArraySize = ARRAYSIZE(featureArray);

	DXGI_SWAP_CHAIN_DESC swapchainDesc{};
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferDesc.Width = width;
	swapchainDesc.BufferDesc.Height = height;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.OutputWindow = hWnd;
	swapchainDesc.SampleDesc.Count = 4;
	swapchainDesc.SampleDesc.Quality = 4;
	swapchainDesc.Windowed = false;
	//swapchainDesc.Windowed = true;

	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;

#if GRAPHICS_DEBUGGING
	HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
		flags, featureArray, featureArraySize, D3D11_SDK_VERSION, &swapchainDesc,
		&swapchain, &device, &featureLevel, &deviceContext);
#else
	IDXGIAdapter1* adapter = GetBestGpu();
	HRESULT result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
		flags, featureArray, featureArraySize, D3D11_SDK_VERSION, &device, &featureLevel, &deviceContext);

	UINT maxQuality;
	result = device->CheckMultisampleQualityLevels(swapchainDesc.BufferDesc.Format, swapchainDesc.SampleDesc.Count, &maxQuality);
	swapchainDesc.SampleDesc.Quality = maxQuality - 1;
	ReleaseIfExists(deviceContext);
	ReleaseIfExists(device);

	result = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
		flags, featureArray, featureArraySize, D3D11_SDK_VERSION, &swapchainDesc,
		&swapchain, &device, &featureLevel, &deviceContext);

	/*
	D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, featureArray, featureArraySize, D3D11_SDK_VERSION, &device, &featureLevel, &deviceContext);

	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
	{
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		assert(SUCCEEDED(device.As(&dxgiDevice)));
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		assert(SUCCEEDED(dxgiDevice->GetAdapter(&adapter)));
		HRESULT result = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		assert(SUCCEEDED(result));
		dxgiFactory->CreateSwapChain(device.Get(), &swapchainDesc, &swapchain);
	}

	UINT maxQuality;
	HRESULT result = device->CheckMultisampleQualityLevels(swapchainDesc.BufferDesc.Format, swapchainDesc.SampleDesc.Count, &maxQuality);
*/
//IDXGISwapChain::CreateSwapChain
//dxgiFactory->CreateSwapChain(device, &swapchainDesc, &swapchain);
#endif
	SetDebugName(device, "Device");
	SetDebugName(swapchain, "Swapchain");
	SetDebugName(deviceContext, "DeviceContext");


	if (result != S_OK)
		exit(1);

//#ifdef _DEBUG
//	result = device->QueryInterface(IID_PPV_ARGS(&debug));
//
//	if (result != S_OK)
//		exit(2);
//#endif

	Pipeline = new pipeline_state_t();
	//basicRendering = new sub_pipeline_state_t();
	//fontRendering = new sub_pipeline_state_t();
	//ID3D11Texture2D* texture = 0;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&Pipeline->render_target_texture);
	device->CreateRenderTargetView(Pipeline->render_target_texture, nullptr, &Pipeline->render_target);
	SetDebugName(Pipeline->render_target_texture, "Pipeline Render Target View Texture");
	SetDebugName(Pipeline->render_target, "Pipeline Render Target View");

	//texture->Release();

	deviceContext->OMSetRenderTargets(1, &Pipeline->render_target, nullptr);

	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	deviceContext->RSSetViewports(1, &viewport);

	D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	device->CreateRasterizerState(&desc, &(Pipeline->rasterState));
	SetDebugName(Pipeline->rasterState, "Default Raster State");
	desc.FillMode = D3D11_FILL_WIREFRAME;
	desc.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&desc, &(Pipeline->debugRasterState));
	SetDebugName(Pipeline->debugRasterState, "Debug Raster State");

	//-----------------------------------------------------------
	defaultShader = LoadShader("Trivial");
	animamationShader = LoadShader("Animated");
	texturedShader = LoadShader("Simple_Texture");
	guiShader = LoadShader("Gui");

	ErrorTexture = DataManager::GetInstance()->LoadTexture("ErrorTexture.png");

	CD3D11_BUFFER_DESC guiBufferDesc(sizeof(glm::vec4), D3D11_BIND_CONSTANT_BUFFER);
	result = device->CreateBuffer(&guiBufferDesc, nullptr, &m_guiBuffer);
	if (result != S_OK)
		exit(3);
	SetDebugName(m_guiBuffer, "Gui Buffer");

	CD3D11_BUFFER_DESC fontBufferDesc(sizeof(RenderableChar) * 64, D3D11_BIND_CONSTANT_BUFFER);
	result = device->CreateBuffer(&fontBufferDesc, nullptr, &m_fontBuffer);
	if (result != S_OK)
		exit(3);
	SetDebugName(m_fontBuffer, "Font Buffer");

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	result = device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);
	if (result != S_OK)
		exit(3);
	SetDebugName(m_constantBuffer, "Constant Buffer");

	{//Line Shit
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));

		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = (UINT)(sizeof(Mesh::Vertex) * LINE_LIMIT * 2);
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &m_LineBuffer);
		SetDebugName(m_LineBuffer, "Line Buffer");

		lines.reserve(LINE_LIMIT * 2);
	}

	glm::vec3 EyePosition = { 1.0f, 0.0f, 0.0f };
	glm::vec3 FocusPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 UpDirection = { 0.0f, 1.0f, 0.0f };
	glm::mat4 LookAt = glm::lookAtLH(EyePosition, FocusPosition, UpDirection);
	m_constantBufferData.view = glm::transpose(LookAt);
	m_constantBufferData.projection = glm::transpose(glm::perspectiveFovLH(70.0f * 3.14159f / 180.0f, (float)width, (float)height, 0.01f, 1000.0f));

	glm::mat4 temp = glm::mat4(1.0f);

	D3D11_BLEND_DESC omDesc;
	ZeroMemory(&omDesc, sizeof(D3D11_BLEND_DESC));
	omDesc.RenderTarget[0].BlendEnable = true;
	omDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	omDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	omDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	omDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	omDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	omDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	omDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&omDesc, &blendState);
	SetDebugName(blendState, "Blend state");

	deviceContext->OMSetBlendState(blendState, 0, 0xffffffff);

	//------------Depth Stuff--------------------------------------------

	D3D11_TEXTURE2D_DESC depthTextureDesc;
	ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
	depthTextureDesc.Width = width;
	depthTextureDesc.Height = height;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.SampleDesc.Count = swapchainDesc.SampleDesc.Count;
	depthTextureDesc.SampleDesc.Quality = swapchainDesc.SampleDesc.Quality;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;


	ID3D11Texture2D *DepthStencilTexture;
	result = device->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);
	if (result != S_OK)
		exit(4);
	SetDebugName(DepthStencilTexture, "DepthStencilTexture");

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace = depthStencilDesc.FrontFace;

	device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState2D);

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
	device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState3D);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = depthTextureDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	result = device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &(Pipeline->depthStencilView));
	SetDebugName(Pipeline->depthStencilView, "Pipeline->depthStencilView");

	DepthStencilTexture->Release();

	if (result != S_OK)
		exit(5);

	deviceContext->OMSetRenderTargets(1, &Pipeline->render_target, Pipeline->depthStencilView);

	SetupLights();
}

#include "FontManager.h"

Renderer::~Renderer()
{
	for (auto r : renderables)
	{
		ReleaseIfExists(r.second->m_VertexBuffer);
		ReleaseIfExists(r.second->m_IndexBuffer);
	}
	renderables.clear();

	for (auto r : external_renderables)
	{
		ReleaseIfExists(r->m_VertexBuffer);
		ReleaseIfExists(r->m_IndexBuffer);
	}
	external_renderables.clear();

	for (auto s : shaders)
	{
		ReleaseIfExists(s);
	}
	shaders.clear();

	DataManager::GetInstance()->ClearShaders();

	for (auto t : textures)
	{
		ReleaseIfExists(t);
	}
	shaders.clear();

	for (auto b : buffers)
	{
		ReleaseIfExists(b->buffer);
		delete b;
	}
	buffers.clear();

	for (auto batch : m_Batcher)
	{
		batch.second->clear();
		delete batch.second;
	}

	delete CharMesh;

	ReleaseIfExists(Pipeline->depthStencilView);
	ReleaseIfExists(Pipeline->depthStencilBuffer);
	ReleaseIfExists(Pipeline->depthStencilState);
	ReleaseIfExists(Pipeline->rasterState);
	ReleaseIfExists(Pipeline->debugRasterState);
	ReleaseIfExists(Pipeline->render_target);
	ReleaseIfExists(Pipeline->render_target_texture);

	ReleaseIfExists(blendState);
	ReleaseIfExists(depthStencilState2D);
	ReleaseIfExists(depthStencilState3D);
	delete Pipeline;

	ReleaseIfExists(m_constantBuffer);
	ReleaseIfExists(m_fontBuffer);
	ReleaseIfExists(m_guiBuffer);
	ReleaseIfExists(m_LineBuffer);

//#ifdef _DEBUG
//	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
//	ReleaseIfExists(debug);
//#endif

	FontManager::GetInstance()->CleanUp();
	delete FontManager::GetInstance();

	ReleaseIfExists(deviceContext);
	ReleaseIfExists(swapchain);
	ReleaseIfExists(device);
}

int Renderer::LoadShader(char * name)
{
	if (shadersFromName.find(name) != shadersFromName.end())
		return shadersFromName[name];
	DataManager* dm = DataManager::GetInstance();

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_1_", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_2_", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_3_", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_4_", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_5_", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "EXTRA_6_", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	size_t fileNameLen = strlen(name) + 9;
	ShaderProgram* shaderProgram = new ShaderProgram();

	char* vertexName = new char[fileNameLen];
	sprintf_s(vertexName, fileNameLen, "%s_VS.cso", name);
	size_t vid = dm->LoadVertexShader(vertexName, device, shaderProgram->m_InputLayout, inputElementDesc, ARRAYSIZE(inputElementDesc));
	if (vid == LOADERROR) Utils::PrintError("failed to load %s\n", vertexName);
	else shaderProgram->m_VertexShader = (ID3D11VertexShader*)dm->GetShader(vid);
	SetDebugName(shaderProgram->m_InputLayout, "(%s) Shader Input Layout", name);
	SetDebugName(shaderProgram->m_VertexShader, "(%s) Vertex Shader", name);

	char* pixelName = new char[fileNameLen];
	sprintf_s(pixelName, fileNameLen, "%s_PS.cso", name);
	size_t pid = dm->LoadPixelShader(pixelName, device);
	if (pid == LOADERROR) Utils::PrintError("failed to load %s\n", pixelName);
	else shaderProgram->m_PixelShader = (ID3D11PixelShader*)dm->GetShader(pid);
	SetDebugName(shaderProgram->m_PixelShader, "(%s) Pixel Shader", name);

	shaders.push_back(shaderProgram);

	delete[] pixelName;
	delete[] vertexName;
	int32_t id = (int32_t)(shaders.size() - 1);
	shadersFromName.insert(std::make_pair(name, id));
	return id;
}


size_t Renderer::CreateTexture(const char* name, uint8_t * data, int len, int width, int height)
{
	Texture* texture = new Texture();
	texture->width = width;
	texture->height = height;

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.ArraySize = 1;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.Usage = D3D11_USAGE_DYNAMIC;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.Height = texture->height;
	td.Width = texture->width;
	td.MipLevels = 1;
	td.MiscFlags = 0;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;

	AsyncLoading.lock();
	//ID3D11Texture2D *pTexture = NULL;
	HRESULT result = device->CreateTexture2D(&td, NULL, &texture->pTexture);
	SetDebugName(texture->pTexture, "(%s) Texture pTexture", name);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(texture->pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
	for (int i = 0; i < height; ++i)
	{
		memcpy(mappedData, data, texture->width * 4);
		mappedData += mappedResource.RowPitch;
		data += texture->width * 4;
	}

	deviceContext->Unmap(texture->pTexture, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = td.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	//ID3D11ShaderResourceView* textureSRV;
	result = device->CreateShaderResourceView(texture->pTexture, &SRVDesc, &texture->textureSRV);
	SetDebugName(texture->textureSRV, "(%s) Texture Shader Resource View", name);

	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.Filter = D3D11_FILTER_ANISOTROPIC;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 16;
	sd.ComparisonFunc = D3D11_COMPARISON_EQUAL;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState *pSampler = NULL;
	result = device->CreateSamplerState(&sd, &pSampler);
	AsyncLoading.unlock();
	if (result != S_OK)
		Utils::PrintWarning("There was a problem creating the sampler state for %s!\n", name);
	SetDebugName(pSampler, "(%s) Texture Sampler", name);
	texture->pSampler = pSampler;
	textures.push_back(texture);

	texture->m_Id = textures.size() - 1;
	return textures.size() - 1;
}

Texture* Renderer::GetTexture(size_t id)
{
	return textures[id];
}

Buffer* Renderer::CreateBuffer(size_t size)
{
	Buffer* buffer = new Buffer();
	CD3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(CD3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT result = device->CreateBuffer(&bufferDesc, nullptr, &buffer->buffer);
	SetDebugName(buffer->buffer, "{%d} Buffer buffer {%p}", size, (void*)buffer);
	if (result == S_OK)
	{
		buffer->size = size;
		buffers.push_back(buffer);
		return buffer;
	}
	delete buffer;
	return nullptr;
}

void Renderer::UseShader(int i)
{
	if (!Utils::isInRange(-1, (int)shaders.size() - 1, i))
	{
		printf("Shader index out of range");
		return;
	}
	if (i == -1)
	{
		deviceContext->IASetInputLayout(shaders[defaultShader]->m_InputLayout);
		deviceContext->VSSetShader(shaders[defaultShader]->m_VertexShader, nullptr, 0);
		deviceContext->PSSetShader(shaders[defaultShader]->m_PixelShader, nullptr, 0);
		return;
	}
	deviceContext->IASetInputLayout(shaders[i]->m_InputLayout);
	deviceContext->VSSetShader(shaders[i]->m_VertexShader, nullptr, 0);
	deviceContext->PSSetShader(shaders[i]->m_PixelShader, nullptr, 0);
}

void Renderer::UseTexture(size_t i)
{
	if (!Utils::isInRange((size_t)0, textures.size() - 1, i))
	{
		//Utils::PrintError("Texture index out of range\n");
		deviceContext->PSSetShaderResources(0, 1, &textures[ErrorTexture]->textureSRV);
		deviceContext->PSSetSamplers(0, 1, &textures[ErrorTexture]->pSampler);
		return;
	}
	deviceContext->PSSetShaderResources(0, 1, &textures[i]->textureSRV);
	deviceContext->PSSetSamplers(0, 1, &textures[i]->pSampler);
}

void Renderer::Clear(uint32_t buffer)
{
	//float color[4] = { 0.180f, 0.545f, 0.341f, 1.0f };
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (buffer & BUFFER_DEPTH && Pipeline->depthStencilView)
		deviceContext->ClearDepthStencilView(Pipeline->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	if (buffer & BUFFER_STENCIL && Pipeline->depthStencilView)
		deviceContext->ClearDepthStencilView(Pipeline->depthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
	if (buffer & BUFFER_COLOR && Pipeline->render_target)
		deviceContext->ClearRenderTargetView(Pipeline->render_target, color);
}

Mesh* Renderer::CreateRenderable(Mesh* mesh)
{
	static std::mutex MCreateRenderable;
	if (!mesh)
		return nullptr;

	if (mesh->m_VertexBuffer)
		return mesh;

	if (!MCreateRenderable.try_lock())
		return nullptr;

	std::vector<Mesh::Vertex> verts;
	std::vector<uint32_t> indicies;

	for (uint32_t i = 0; i < mesh->m_NumIndicies; ++i)
		indicies.push_back(mesh->m_IndexBufferInfo[i]);
	for (uint32_t i = 0; i < mesh->m_NumVerticies; ++i)
		verts.push_back(mesh->m_VertexBufferInfo[i]);
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)(sizeof(Mesh::Vertex) * verts.size());
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &mesh->m_VertexBuffer);
	SetDebugName(mesh->m_VertexBuffer, "Mesh Vertex Buffer {%p}", (void*)mesh);

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	deviceContext->Map(mesh->m_VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
	memcpy(MappedSubresource.pData, verts.data(), sizeof(Mesh::Vertex) * verts.size());
	deviceContext->Unmap(mesh->m_VertexBuffer, NULL);

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)(sizeof(uint32_t) * indicies.size());
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bufferDesc, NULL, &mesh->m_IndexBuffer);
	SetDebugName(mesh->m_IndexBuffer, "Mesh Index Buffer {%p}", (void*)mesh);

	ZeroMemory(&MappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	deviceContext->Map(mesh->m_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
	memcpy(MappedSubresource.pData, indicies.data(), sizeof(uint32_t) * indicies.size());
	deviceContext->Unmap(mesh->m_IndexBuffer, NULL);

	verts.clear();
	indicies.clear();
	mesh->m_Type = TRIANGLE_LIST;

	//renderables.insert(std::make_pair(name, mesh));
	external_renderables.push_back(mesh);
	MCreateRenderable.unlock();
	return mesh;
}

uint32_t Renderer::GetWidth()
{
	return width;
}

uint32_t Renderer::GetHeight()
{
	return height;
}

uint32_t Renderer::GetRealWidth()
{
	RECT rect;
	GetClientRect(m_Window, &rect);
	return abs(rect.right - rect.left);
}

uint32_t Renderer::GetRealHeight()
{
	RECT rect;
	GetClientRect(m_Window, &rect);
	return abs(rect.bottom - rect.top);
}

float Renderer::GetInvWidth()
{
	static float invWidth = 1.0f / (float)width;
	return invWidth;
}

float Renderer::GetInvHeight()
{
	static float invHeight = 1.0f / (float)height;
	return invHeight;
}

ID3D11Device * Renderer::GetDevice()
{
	return device;
}

ID3D11DeviceContext * Renderer::GetDeviceContext()
{
	return deviceContext;
}

/*
using namespace glm;
void Renderer::RenderDebug()
{
	if (m_wireframe)
		deviceContext->RSSetState(Pipeline->debugRasterState);
	else
		deviceContext->RSSetState(Pipeline->rasterState);

	//deviceContext->ClearDepthStencilView(Pipeline->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//const float RGBA[4] = { 0.180f, 0.545f, 0.341f, 1.0f };
	//deviceContext->ClearRenderTargetView(renderTargetView, RGBA);
	deviceContext->PSSetShader(Pipeline->pixel_shader, NULL, 0);
	deviceContext->VSSetShader(Pipeline->vertex_shader, NULL, 0);
	deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;

	for (auto renderable : renderables)
	{
		switch (renderable.second->type)
		{
		case TRIANGLE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case LINE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		}
		m_constantBufferData.model = glm::mat4(1);
		deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

		deviceContext->IASetVertexBuffers(0, 1, &renderable.second->vertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(renderable.second->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexedInstanced(renderable.second->m_NumIndicies, 1, 0, 0, 0);
	}
	swapchain->Present(0, 0);
}
*/

void Renderer::RenderText(Font* font, std::string str, float x, float y)
{
	Char* prevChar = nullptr;
	int pos = 0;
	float _x = x * 2 - 1;
	float _y = y * 2;
	float inv2Width = 2.0f * GetInvWidth();
	float inv2Height = 2.0f * GetInvHeight();
	float invFontWidth = 1.0f / font->width;
	float invFontHeight = 1.0f / font->height;
	for (size_t pos = 0; pos < str.length(); pos += 64)
	{
		StringRenderable renderable;
		for (size_t i = pos; i < str.length() && (i - pos) < 64; ++i)
		{
			char c = str.c_str()[i];
			Char* ch = font->characters[c];
			if (ch != nullptr)
			{
				_x += font->GetKerning(prevChar, ch) * inv2Width;
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
				rc.z = font->maxHeight*inv2Height;
				rc.w = 1.0f;
				renderable.chars.push_back(rc);
				prevChar = ch;
			}
		}
		while (renderable.chars.size() < 64)
		{
			RenderableChar rc;
			ZeroMemory(&rc, sizeof(rc));
			renderable.chars.push_back(rc);
		}


		AsyncLoading.lock();
		Mesh* mesh = createPlane();
		deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UseShader(font->shader);

		deviceContext->UpdateSubresource(m_fontBuffer, 0, NULL, renderable.chars.data(), 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &m_fontBuffer);

		UseTexture(font->texture);

		UINT stride = sizeof(Mesh::Vertex);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &mesh->m_VertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(mesh->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->DrawIndexedInstanced((UINT)mesh->m_NumIndicies, (UINT)renderable.chars.size(), 0, 0, 0);
		AsyncLoading.unlock();
	}
}

void Renderer::RenderImage(size_t textureId, float x, float y, float sx, float sy)
{
	Texture* tex = (Texture*)Renderer::GetInstance()->GetTexture(textureId);
	if (!tex)
		return;
	float invImgWidth = (tex->width * sx) * GetInvWidth();
	float invImgHeight = (tex->height * sy) * GetInvHeight();
	float _x = x * 2 - 1;
	float _y = y * 2;

	glm::vec4 pos = glm::vec4(_x, _y, invImgWidth * 2.0f, invImgHeight * 2.0f);

	AsyncLoading.lock();
	Mesh* mesh = createPlane();
	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UseShader(guiShader);

	deviceContext->UpdateSubresource(m_guiBuffer, 0, NULL, &pos[0], 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_guiBuffer);

	UseTexture(textureId);

	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &mesh->m_VertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(mesh->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed((UINT)mesh->m_NumIndicies, 0, 0);
	AsyncLoading.unlock();
}

void Renderer::RenderImageResource(uint32_t resourceId, float x, float y, float sx, float sy)
{
	Texture* tex = (Texture*)DataManager::GetInstance()->Get(resourceId, Data::DataType::TextureData);
	RenderImage(tex->m_Id, x, y, sx, sy);
}

void Renderer::SetCameraMatrix(glm::mat4 val)
{
	glm::mat4 view = glm::inverse(val);
	m_constantBufferData.view = glm::transpose(view);
	glm::mat3 rotMat(view);
	glm::vec3 d(view[3]);
	glm::vec3 retVec = -d * rotMat;
	AudioManager::GetInstance()->SetListenerPos(-d * rotMat);
	AudioManager::GetInstance()->SetListenerOrientation(view[2], view[1]);
}

glm::mat4 Renderer::GetCameraMatrix()
{
	return glm::inverse(m_constantBufferData.view);
}

void Renderer::RenderGuiElements()
{
	deviceContext->OMSetDepthStencilState(depthStencilState2D, 0);
	Canvas* canvas = ObjectManager::GetInstance()->GetCanvas();
	if (canvas)
		for (auto element : canvas->GetElements())
			RenderGuiElement(element);
	deviceContext->OMSetDepthStencilState(depthStencilState3D, 0);
}

void Renderer::RenderGuiElement(GuiElement* elem)
{
	if (GuiImage* gi = dynamic_cast<GuiImage*>(elem))
		RenderGuiImage(gi);
	else if (GuiText* gt = dynamic_cast<GuiText*>(elem))
		RenderGuiText(gt);
	else if (GuiTextbox* gt = dynamic_cast<GuiTextbox*>(elem))
		RenderGuiTextbox(gt);
}

void Renderer::RenderGuiImage(GuiImage* elem)
{
	glm::vec2 off = elem->GetOffset();
	glm::vec2 scale = elem->GetScale();
	RenderImage(elem->GetImage(), off.x, off.y, scale.x, scale.y);
}

void Renderer::RenderGuiText(GuiText* elem)
{
	glm::vec2 off = elem->GetOffset();
	RenderText(elem->GetFont(), elem->GetText(), elem->GetOffset().x, elem->GetOffset().y);
}

void Renderer::RenderGuiTextbox(GuiTextbox* elem)
{
	glm::vec2 off = elem->GetOffset();
	RenderImage(elem->GetImage(), off.x, off.y, 1, 1);
	RenderText(elem->GetFont(), elem->GetText(), elem->GetTextOffset().x, elem->GetTextOffset().y);
}

//void Renderer::AddGuiElement(GuiElement * element)
//{
//	m_GuiElements.push_back(element);
//}

Mesh* Renderer::createPlane()
{
	if (CharMesh)
		return CharMesh;

	Mesh* mesh = new Mesh();
	Mesh::Vertex verts[] = {
		Mesh::Vertex({ 1.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }),
		Mesh::Vertex({ 0.0f, 1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }),
		Mesh::Vertex({ 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }),
		Mesh::Vertex({ 0.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f }),
	};
	uint32_t indicies[] = { 2,0,1, 3,2,1 };

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)(sizeof(Mesh::Vertex) * ARRAYSIZE(verts));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &mesh->m_VertexBuffer);
	SetDebugName(mesh->m_VertexBuffer, "Plane Vertex Buffer");

	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	deviceContext->Map(mesh->m_VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
	memcpy(MappedSubresource.pData, verts, sizeof(Mesh::Vertex) * ARRAYSIZE(verts));
	deviceContext->Unmap(mesh->m_VertexBuffer, NULL);

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)(sizeof(uint32_t) * ARRAYSIZE(indicies));
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&bufferDesc, NULL, &mesh->m_IndexBuffer);
	SetDebugName(mesh->m_VertexBuffer, "Plane Index Buffer");

	ZeroMemory(&MappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	deviceContext->Map(mesh->m_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
	memcpy(MappedSubresource.pData, indicies, sizeof(uint32_t) * ARRAYSIZE(indicies));
	deviceContext->Unmap(mesh->m_IndexBuffer, NULL);

	mesh->m_Type = TRIANGLE_LIST;
	mesh->m_NumVerticies = 4;
	mesh->m_NumIndicies = 6;

	CharMesh = mesh;
	external_renderables.push_back(CharMesh);

	return mesh;
}

void Renderer::RenderMesh(Mesh * renderable, glm::mat4 transform = glm::mat4(1))
{

	switch (renderable->m_Type)
	{
		case TRIANGLE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case LINE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
	}
	m_constantBufferData.data[0].m_Model = glm::transpose(transform);
	m_constantBufferData.data[0].m_Color = glm::vec4(1.0f);
	deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_LightBuffer->UseBufferPS(0);

	UseShader(renderable->m_Shader);
	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &renderable->m_VertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(renderable->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexedInstanced(renderable->m_NumIndicies, 1, 0, 0, 0);
}

void Renderer::RenderObject(Object * obj)
{
	MeshComponent* mc = obj->GetComponent<MeshComponent>();
	if (!mc) return;
	Mesh* renderable = mc->GetRenderable(this);
	if (!renderable) return;
	switch (renderable->m_Type)
	{
		case TRIANGLE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case LINE_LIST:
			deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
	}

	m_constantBufferData.data[0].m_Model = glm::transpose(obj->GetWorldMatrix());
	m_constantBufferData.data[0].m_Color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	MaterialComponent* mat = obj->m_MaterialComp;
	if(mat) m_constantBufferData.data[0].m_Color = mat->m_Color;

	deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	m_LightBuffer->UseBufferPS(0);

	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;

	UseShader(renderable->m_Shader);

	if (obj->m_MeshComp->GetMesh()->m_Texture >= 0)
	{
		UseTexture(obj->m_MeshComp->GetMesh()->m_Texture);
		UseShader(texturedShader);
	}
	AnimationComponent* Acomp = obj->m_AnimationComp;
	if (Acomp != NULL)
	{
		glm::mat4 Bones[64];
		unsigned int count = Acomp->getNumInterpolatedMatricies();
		for (unsigned int i = 0; i < count && i < 64; ++i)
			Bones[i] = glm::transpose(Acomp->getInterpolatedMatrices()[i]);

		static Buffer* AnimatedFrame = nullptr;
		if (!AnimatedFrame) AnimatedFrame = CreateBuffer(sizeof(glm::mediump_fmat4x4) * 64);
		AnimatedFrame->WriteData((void*)Bones);

		static Buffer* InvBindPos = nullptr;
		for (unsigned int i = 0; i < renderable->m_NumBones && i < 64; ++i)
			Bones[i] = glm::transpose(renderable->m_BoneOffsets[i]);
		if (!InvBindPos) InvBindPos = CreateBuffer(sizeof(glm::mediump_fmat4x4) * 64);
		InvBindPos->WriteData((void*)Bones);

		deviceContext->VSSetConstantBuffers(1, 1, &AnimatedFrame->buffer);
		deviceContext->VSSetConstantBuffers(2, 1, &InvBindPos->buffer);

		UseShader(animamationShader);
	}
	deviceContext->IASetVertexBuffers(0, 1, &renderable->m_VertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(renderable->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexedInstanced(renderable->m_NumIndicies, 1, 0, 0, 0);
}

void Renderer::BatchObject(Object * obj)
{
	MeshComponent* mc = obj->m_MeshComp;
	if (!mc) return;
	Mesh* renderable = mc->GetRenderable(this);
	if (!renderable) return;

	//static std::mutex MExists;
	//
	//std::vector<mat4>* list = nullptr;
	//std::unordered_map<Mesh*, std::vector<glm::mat4>*>::iterator exists;
	//while ((exists = m_Batcher.find(renderable))==m_Batcher.end())
	//{
	//	if (!MExists.try_lock())
	//		continue;
	//	list = new std::vector<mat4>(32);
	//	m_Batcher.insert(std::make_pair(renderable, list));
	//	MExists.unlock();
	//	break;
	//}
	//if(list==nullptr)
	//	list = m_Batcher.find(renderable)->second;
	/*
	static std::mutex MExists;

	std::vector<mat4>* list = nullptr;
	while (m_Batcher.find(renderable) == m_Batcher.end())
	{
		MExists.lock();
		if (m_Batcher.find(renderable) != m_Batcher.end())
		{
			MExists.unlock();
			break;
		}
		list = new std::vector<mat4>(BATCH_SIZE);
		m_Batcher.insert(std::make_pair(renderable, list));
		MExists.unlock();
		break;
	}
	if (list == nullptr)
		list = m_Batcher.find(renderable)->second;
		*/
		/*
		static std::mutex MExists;
		std::vector<mat4>* list;
	FailedToLock:
		auto exists = m_Batcher.find(renderable);
		if (exists == m_Batcher.end())
		{
			if (!MExists.try_lock())
				goto FailedToLock;
			list = new std::vector<mat4>(32);
			m_Batcher.insert(std::make_pair(renderable, list));
			MExists.unlock();
		}
		else
			list = exists->second;
			*/
			//static std::mutex VecPush;
			//VecPush.lock();
	std::vector<PerObjectBatchData>* list = nullptr;
	if (m_Batcher.find(renderable) == m_Batcher.end())
	{
		list = new std::vector<PerObjectBatchData>(BATCH_SIZE);
		m_Batcher.insert(std::make_pair(renderable, list));
	}
	else
		list = m_Batcher.find(renderable)->second;
	glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	MaterialComponent* mat = obj->m_MaterialComp;
	if (mat) color = mat->m_Color;
	list->push_back({ glm::transpose(obj->m_Transform), color });
	//VecPush.unlock();
}

void Renderer::BatchObjects(std::list<Object*>::iterator begin, std::list<Object*>::iterator end)
{
	for (auto it = begin; it != end; ++it)
		BatchObject(*it);
}

void Renderer::RenderBatch()
{
	for (auto batch : m_Batcher)
	{
		for (size_t i = 0; i < batch.second->size(); i += BATCH_SIZE)
		{
			size_t count = std::min((size_t)BATCH_SIZE, batch.second->size() - i);
			memcpy(m_constantBufferData.data, batch.second->data() + i, sizeof(PerObjectBatchData)*count);
			Mesh* renderable = batch.first;

			UseShader(renderable->m_Shader);

			if (renderable->m_Texture >= 0)
			{
				UseTexture(renderable->m_Texture);
				UseShader(texturedShader);
			}

			switch (renderable->m_Type)
			{
				case TRIANGLE_LIST:
					deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					break;
				case LINE_LIST:
					deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
					break;
			}

			deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
			deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
			m_LightBuffer->UseBufferPS(0);

			UINT stride = sizeof(Mesh::Vertex);
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &renderable->m_VertexBuffer, &stride, &offset);
			deviceContext->IASetIndexBuffer(renderable->m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			deviceContext->DrawIndexedInstanced((UINT)renderable->m_NumIndicies, (UINT)count, 0, 0, 0);
		}
		batch.second->clear();
	}
}

/*
void Renderer::DebugRender(ObjectManager * objManager)
{
	size_t id = GenerateAABBMesh();
	DataManager* dm = DataManager::GetInstance();
	Mesh* mesh = (Mesh*)dm->Get(id);
	for (Object* obj : objManager->GetObjectsWithComponent<AABBCollider>())
	{
		RenderMesh(mesh, obj->m_Transform);
	}
}
*/
extern glm::vec3 ExtractCameraPos(const glm::mat4 & a_modelView);

void Renderer::RenderScene(ObjectManager * objManager)
{
	AsyncLoading.lock();
	deviceContext->RSSetState(Pipeline->rasterState);

	//deviceContext->ClearDepthStencilView(Pipeline->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//*/

	UpdateLights();

	//if(!GetAsyncKeyState(VK_TAB))
	m_Frustum.BuildFrustum(70.0f * 3.14159f / 180.0f, 0.1f, 100.0f, (float)width / (float)height, m_constantBufferData.view);
	//m_Frustum.DrawLines();

	QuadTree* quadtree = objManager->m_StaticObjectQuadTree;
	if (quadtree)
	{
		std::vector<Object*> objects = quadtree->Retrieve(&m_Frustum);
		//printf("object count: %d\n", objects.size());
		for (Object* obj : objects)
		{
			if (obj->isEnabled() && obj->m_MeshComp && obj->m_MeshComp->GetMesh())//&& Object::ObjectTags::Player == obj->tag)
			{
				if (!obj->m_MeshComp->GetMesh()->m_Batchable)
					RenderObject(obj);
				else
					BatchObject(obj);
			}
		}
	}
	for (Object* obj : objManager->GetNonStaticCollidableObjects())
	{
		if (obj->isEnabled())//&& Object::ObjectTags::Player == obj->tag)
		{
			if (obj->m_MeshComp && obj->m_MeshComp->GetMesh())
			{
				if (!obj->m_MeshComp->GetMesh()->m_Batchable)
					RenderObject(obj);
				else
					BatchObject(obj);
			}
		}
	}

	RenderBatch();
	AsyncLoading.unlock();
	/*
	ObjectManager* manager = ObjectManager::GetInstance();
	QuadTree* tree = manager->m_StaticObjectQuadTree;
	if (tree)
	{
		glm::mat4 modelView = m_constantBufferData.view;
		glm::vec4 pos = glm::vec4(0, 0, 0, 1);
		glm::vec4 viewDir = glm::normalize(glm::vec4(1, 1, 0, 0));
		viewDir.w = 0;
		vec4 hit;
		tree->Retrieve(pos, viewDir, hit);
	}
	*/
}

void Renderer::AddLine(glm::vec4 start, glm::vec4 end, glm::vec4 color)
{
	if (lines.size() < LINE_LIMIT * 2)
	{
		dirtyLines = true;
		Mesh::Vertex _start, _end;

		ZeroMemory(&_start, sizeof(_start));
		_start.Position = start;
		_start.UV = color;
		lines.push_back(_start);

		ZeroMemory(&_end, sizeof(_end));
		_end.Position = end;
		_end.UV = color;
		lines.push_back(_end);
	}
}

void Renderer::AddLine(glm::vec3 start, glm::vec3 end, glm::vec4 color)
{
	if (lines.size() < LINE_LIMIT * 2)
	{
		dirtyLines = true;
		Mesh::Vertex _start, _end;

		ZeroMemory(&_start, sizeof(_start));
		_start.Position = glm::vec4(start, 1);
		_start.Normal = color;
		lines.push_back(_start);

		ZeroMemory(&_end, sizeof(_end));
		_end.Position = glm::vec4(end, 1);
		_end.Normal = color;
		lines.push_back(_end);
	}
}

void Renderer::ClearLines()
{
	lines.clear();
	dirtyLines = true;
}

void Renderer::RenderDebugData()
{
	if (lines.size() == 0)
		return;

	if (dirtyLines)
	{
		D3D11_MAPPED_SUBRESOURCE MappedSubresource;
		deviceContext->Map(m_LineBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &MappedSubresource);
		memcpy(MappedSubresource.pData, lines.data(), sizeof(Mesh::Vertex) * lines.size());
		deviceContext->Unmap(m_LineBuffer, NULL);
	}

	UseShader(defaultShader);

	deviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	m_constantBufferData.data[0].m_Model = glm::mat4(1);
	m_constantBufferData.data[0].m_Color = glm::vec4(1, 1, 1, -1);
	deviceContext->UpdateSubresource(m_constantBuffer, 0, NULL, &m_constantBufferData, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_LineBuffer, &stride, &offset);
	deviceContext->Draw(lines.size(), 0);
}

#include "InputManager.h"

bool temp = false;

void Renderer::Present()
{
	static int i = 1;
	if (InputManager::GetInstance()->IsKeyDown(VK_F1))
		i = 1 - i;
	swapchain->Present(i, 0);
}

void Renderer::SetSize(int _width, int _height)
{
	static std::mutex ResizeMutex;
	ResizeMutex.lock();

	ResizeMutex.unlock();
}

#undef ReleaseIfExists

void Buffer::UseBufferVS(size_t slot)
{
	ID3D11DeviceContext* deviceContext = Renderer::GetInstance()->GetDeviceContext();
	deviceContext->VSSetConstantBuffers(slot, 1, &this->buffer);
}

void Buffer::UseBufferPS(size_t slot)
{
	ID3D11DeviceContext* deviceContext = Renderer::GetInstance()->GetDeviceContext();
	deviceContext->PSSetConstantBuffers(slot, 1, &this->buffer);
}

void Buffer::WriteData(void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ID3D11DeviceContext* deviceContext = Renderer::GetInstance()->GetDeviceContext();
	deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, data, size);
	deviceContext->Unmap(buffer, 0);
}

void Buffer::WriteData(uint32_t size, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ID3D11DeviceContext* deviceContext = Renderer::GetInstance()->GetDeviceContext();
	deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, data, size);
	deviceContext->Unmap(buffer, 0);
}

void Buffer::ClearAndWriteData(uint32_t size, void* data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ID3D11DeviceContext* deviceContext = Renderer::GetInstance()->GetDeviceContext();
	deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memset(mappedResource.pData, 0, this->size);
	memcpy(mappedResource.pData, data, size);
	deviceContext->Unmap(buffer, 0);
}