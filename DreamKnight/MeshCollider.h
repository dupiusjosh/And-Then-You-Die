#pragma once
#include "Collider.h"
#include "AABBCollider.h"
struct Triangle
{
	Triangle(glm::vec3 _a, glm::vec3 _b, glm::vec3 _c, glm::vec3 _normal)
	{
		a = _a;
		b = _b;
		c = _c;
		m_Normal = _normal;
	}

	union
	{
		struct { glm::vec3 a, b, c; };
		glm::vec3 m_Vertices[3];
	};

	glm::vec3 m_Normal;
};

class MeshCollider : public CollidableComponent
{
private:
	AABBCollider m_bounds;
	Mesh* m_mesh;
	bool m_built;

public:
	MeshCollider();
	MeshCollider(Mesh* mesh);

	std::vector<Triangle> triangles;
	void OnStart() override;
	void OnUpdate(float deltaTime) override;
	void BuildTriangles();

	const Mesh& GetMesh();
	AABBCollider& GetBounds();

	~MeshCollider();
};