#pragma once
#include "Component.h"
#include "Renderer.h"

struct MeshComponent : public Component
{
private:
	Mesh* m_mesh = nullptr;

public:
	MeshComponent(Mesh* mesh);
	~MeshComponent();
	void OnUpdate(float deltaTime);
	Mesh* GetRenderable(Renderer* renderer);
	Mesh* GetMesh();
	void SetMesh(Mesh* newMesh);
};