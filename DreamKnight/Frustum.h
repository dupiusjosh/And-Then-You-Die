#pragma once
#include <math.h>
#define GLM_FORCE_SWIZZLE
#include "glm.h"
#include "AABBCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"

class Frustum
{
	struct Plane
	{
		glm::vec3 normal;
		float offset;
	};
	enum FrustumCorners { FTL = 0, FBL, FBR, FTR, NTL, NTR, NBR, NBL };
	enum FrustumPlanes { NEAR_PLANE = 0, FAR_PLANE, LEFT_PLANE, RIGHT_PLANE, TOP_PLANE, BOTTOM_PLANE };

	Plane planes[6];
	glm::vec3 corners[8];
	int ClassifyToPlane(const Plane& plane, glm::vec3& point) const;
	int ClassifyToPlane(const Plane& plane, AABBCollider& aabb) const;
	int ClassifyToPlane(const Plane& plane, SphereCollider& sphere) const;
	int ClassifyToPlane(const Plane& plane, CapsuleCollider& capsule) const;
public:
	void ComputePlane(Plane &plane, const glm::vec3& pointA, const glm::vec3& pointB, const glm::vec3 &pointC);

	void BuildFrustum(float fov, float nearDist, float farDist, float ratio, const glm::mat4& camXform);

	int FrustumToAABB(AABBCollider & aabb);

	void DrawLines();

};