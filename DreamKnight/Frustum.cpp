#include "Frustum.h"
#include <math.h>

int Frustum::ClassifyToPlane(const Plane & plane, glm::vec3 & point) const
{
	float f = glm::dot(plane.normal, point) - plane.offset;
	return (f == 0) ? 0 : (f > 0) ? 1 : -1; //returns the sign of a float (-1, 0, 1)
}


int Frustum::ClassifyToPlane(const Plane & plane, SphereCollider & sphere) const
{
	glm::vec3 point = sphere.GetWorldPosition();// +(plane.normal*sphere.m_Radius);
	float val = dot(plane.normal, point) - plane.offset;
	if (abs(val) < sphere.GetRadius())
		return 0;
	return (val < 0) ? -1 : 1;
}

int Frustum::ClassifyToPlane(const Plane & plane, AABBCollider & aabb) const
{
	glm::vec3 E = (aabb.GetWorldMax() - aabb.GetWorldMin()) * 0.5f;
	glm::vec3 N = glm::abs(plane.normal);
	float fRadius = glm::dot(N, E);

	glm::vec3 point = glm::vec3(aabb.GetWorldMin()) + E;

	float val = dot(plane.normal, point) - plane.offset;
	if (abs(val) < fRadius)
		return 0;
	return (val < 0) ? -1 : 1;
}

int Frustum::ClassifyToPlane(const Plane & plane, CapsuleCollider & capsule) const
{
	return 0;
}

void Frustum::ComputePlane(Plane &plane, const glm::vec3& pointA, const glm::vec3& pointB, const glm::vec3 &pointC)
{
	plane.normal = glm::cross(pointB - pointA, pointC - pointA);
	plane.normal = normalize(plane.normal);
	plane.offset = glm::dot(plane.normal, pointA);
}

#include "Renderer.h"
#include "DataManager.h"

glm::vec3 ExtractCameraPos(const glm::mat4 & a_modelView)
{
	glm::mat4 modelView = glm::transpose(a_modelView);
	glm::mat3 rotMat(modelView);
	glm::vec3 d(modelView[3]);

	glm::vec3 retVec = -d * rotMat;
	return retVec;
}

void Frustum::BuildFrustum(float fov, float nearDist, float farDist, float ratio, const glm::mat4& camXform)
{
	float temp = tanf(fov / 2.0f);
	float nearH = temp * nearDist;
	float nearW = nearH * ratio;
	float farH = temp * farDist;
	float farW = farH * ratio;

	glm::vec3 pos = ExtractCameraPos(camXform);
	glm::vec3 viewDir = -camXform[2];
	//printf("%f, %f, %f\n", pos.x, pos.y, pos.z);

	glm::vec3 up = camXform[1];
	glm::vec3 w = camXform[0];
	glm::vec3 nearCenter = pos - viewDir * nearDist;
	glm::vec3 farCenter = pos - viewDir * farDist;

	//printf("\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
	//	camXform[0][0], camXform[0][1], camXform[0][2], camXform[0][3],
	//	camXform[1][0], camXform[1][1], camXform[1][2], camXform[1][3],
	//	camXform[2][0], camXform[2][1], camXform[2][2], camXform[2][3],
	//	camXform[3][0], camXform[3][1], camXform[3][2], camXform[3][3]);

	corners[NTL] = nearCenter + (up * nearH) - (w * nearW);
	corners[NTR] = nearCenter + (up * nearH) + (w * nearW);
	corners[NBL] = nearCenter - (up * nearH) - (w * nearW);
	corners[NBR] = nearCenter - (up * nearH) + (w * nearW);
	corners[FTL] = farCenter + (up * farH) - (w * farW);
	corners[FTR] = farCenter + (up * farH) + (w * farW);
	corners[FBL] = farCenter - (up * farH) - (w * farW);
	corners[FBR] = farCenter - (up * farH) + (w * farW);

	//Renderer* r = Renderer::GetInstance();
	//Mesh* sphere = (Mesh*)DataManager::GetInstance()->Get(DataManager::GetInstance()->LoadMesh("sphere.fbx", true));
	//for (int i = 0; i < 8; ++i)
	//	r->RenderMesh(sphere, glm::translate(glm::mat4(1), corners[i]));
	//r->RenderMesh(sphere, glm::translate(glm::mat4(1), pos));

	ComputePlane(planes[NEAR_PLANE], corners[NBR], corners[NBL], corners[NTL]);
	ComputePlane(planes[FAR_PLANE], corners[FBL], corners[FBR], corners[FTR]);
	ComputePlane(planes[LEFT_PLANE], corners[NBL], corners[FBL], corners[FTL]);
	ComputePlane(planes[RIGHT_PLANE], corners[FBR], corners[NBR], corners[NTR]);
	ComputePlane(planes[TOP_PLANE], corners[NTR], corners[NTL], corners[FTL]);
	ComputePlane(planes[BOTTOM_PLANE], corners[NBL], corners[NBR], corners[FBR]);
}

int Frustum::FrustumToAABB(AABBCollider& aabb)
{
	int val = -1;
	for (int i = 0; i < 6; ++i)
	{
		int temp = ClassifyToPlane(planes[i], aabb);
		if (temp == 1)
			return 1;
		val = max(temp, val);
	}
	return val;
}

void Frustum::DrawLines()
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->AddLine(corners[NTL], corners[NTR]);
	renderer->AddLine(corners[NTR], corners[NBR]);
	renderer->AddLine(corners[NBR], corners[NBL]);
	renderer->AddLine(corners[NBL], corners[NTL]);

	renderer->AddLine(corners[NTL], corners[FTL]);
	renderer->AddLine(corners[NTR], corners[FTR]);
	renderer->AddLine(corners[NBR], corners[FBR]);
	renderer->AddLine(corners[NBL], corners[FBL]);

	renderer->AddLine(corners[FTL], corners[FTR]);
	renderer->AddLine(corners[FTR], corners[FBR]);
	renderer->AddLine(corners[FBR], corners[FBL]);
	renderer->AddLine(corners[FBL], corners[FTL]);
}
