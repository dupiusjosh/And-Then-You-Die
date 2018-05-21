#pragma once
#include "glm.h"
#include <vector>
#include <iostream>

#include "Collider.h"
#include "AABBCollider.h"
#include "SphereCollider.h"
#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "RigidBodyComponent.h"
#include "BaseNet_Comp.h"
#include "NetPlayerComponent.h"
#include "Renderer.h"
//#include "QuadTree.h"
//#include "Object.h"
//#include "ObjectManager.h"

namespace CollisionDetection
{
	///////////////////
	// Helper Functions
	///////////////////

	static glm::vec4 ClosestPointOnLine(glm::vec4 lineStart, glm::vec4 lineEnd, glm::vec4 point)
	{
		glm::vec4 ab = lineEnd - lineStart;

		// Project c onto ab, computing the 
		// paramaterized position d(t) = a + t * (b - a)
		float t = glm::dot(point - lineStart, ab) / glm::dot(ab, ab);

		// Clamp T to a 0-1 range. If t was < 0 or > 1
		// then the closest point was outside the line!
		t = glm::clamp(t, FLT_EPSILON, 1.0f);

		glm::vec4 r(lineStart + t * (lineEnd - lineStart));
		r.w = 1.0f;
		// Compute the projected position from the clamped t and return result
		return r;
	}

	static bool LineSegment2Triangle(glm::vec3 &vOut, size_t &uiTriIndex, Triangle *pTris, size_t uiTriCount, const glm::vec3 &vStart, const glm::vec3 &vEnd)
	{
		/*Reminder: Find the NEAREST interesecting triangle*/
		bool found = false;
		glm::vec3 end = vEnd;

		for (size_t i = 0; i < uiTriCount; i++)
		{
			glm::vec3 currentNormal = pTris[i].m_Normal;
			glm::vec3 currPpt = pTris[i].m_Vertices[0];

			if (glm::dot(vStart, pTris[i].m_Normal) - glm::dot(pTris[i].m_Vertices[0], pTris[i].m_Normal) < 0)
				continue;
			if (glm::dot(end, pTris[i].m_Normal) - glm::dot(pTris[i].m_Vertices[0], pTris[i].m_Normal) > 0)
				continue;

			auto d0 = dot(pTris[i].m_Normal, vStart);
			auto d1 = dot(pTris[i].m_Normal, pTris[i].m_Vertices[0]);
			auto d2 = d0 - d1;
			auto l = end - vStart;
			auto d3 = dot(pTris[i].m_Normal, l);
			auto df = -(d2 / d3);

			auto cp = vStart + l * df;

			auto edge0 = pTris[i].m_Vertices[1] - pTris[i].m_Vertices[0];
			glm::vec3 normal0 = glm::cross(edge0, currentNormal);

			auto edge1 = pTris[i].m_Vertices[2] - pTris[i].m_Vertices[1];
			glm::vec3 normal1 = glm::cross(edge1, currentNormal);

			auto edge2 = pTris[i].m_Vertices[0] - pTris[i].m_Vertices[2];
			glm::vec3 normal2 = glm::cross(edge2, currentNormal);

			if (dot((cp - pTris[i].m_Vertices[0]), normal0) > 0)
				continue;

			if (dot((cp - pTris[i].m_Vertices[1]), normal1) > 0)
				continue;

			if (dot((cp - pTris[i].m_Vertices[2]), normal2) > 0)
				continue;

			end = cp;
			vOut = cp;
			uiTriIndex = i;
			found = true;
		}

		return found;
	}

	static glm::vec3 ClosestPointToTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 bc = c - b;
		// Compute parametric position s for projection P’ of P on AB,
		// P’ = A + s*AB, s = snom/(snom+sdenom)
		float snom = glm::dot(p - a, ab), sdenom = glm::dot(p - b, a - b);
		// Compute parametric position t for projection P’ of P on AC,
		// P’ = A + t*AC, s = tnom/(tnom+tdenom)
		float tnom = glm::dot(p - a, ac), tdenom = glm::dot(p - c, a - c);
		if (snom <= 0.0f && tnom <= 0.0f) return a; // Vertex region early out
													// Compute parametric position u for projection P’ of P on BC,
													// P’ = B + u*BC, u = unom/(unom+udenom)
		float unom = glm::dot(p - b, bc), udenom = glm::dot(p - c, b - c);
		if (sdenom <= 0.0f && unom <= 0.0f) return b; // Vertex region early out
		if (tdenom <= 0.0f && udenom <= 0.0f) return c; // Vertex region early out
														// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0

														//Calcs face normal
		glm::vec3 n = glm::cross(b - a, c - a);
		float vc = glm::dot(n, glm::cross(a - p, b - p));
		// If P outside AB and within feature region of AB,
		// return projection of P onto AB
		if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
			return a + snom / (snom + sdenom) * ab;
		// P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
		float va = glm::dot(n, glm::cross(b - p, c - p));
		// If P outside BC and within feature region of BC,
		// return projection of P onto BC
		if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
			return b + unom / (unom + udenom) * bc;
		// P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
		float vb = glm::dot(n, glm::cross(c - p, a - p));
		// If P outside CA and within feature region of CA,
		// return projection of P onto CA
		if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
			return a + tnom / (tnom + tdenom) * ac;
		// P must project inside face region. Compute Q using barycentric coordinates
		float u = va / (va + vb + vc);
		float v = vb / (va + vb + vc);
		float w = 1.0f - u - v; // = vc / (va + vb + vc)
		return u * a + v * b + w * c;
	}

	static glm::vec4 ClosestPointToAABB(glm::vec4 point, AABBCollider* aabb)
	{
		glm::vec4 aabbWorldMin = aabb->GetWorldMin();
		glm::vec4 aabbWorldMax = aabb->GetWorldMax();

		glm::vec4 pointOfCollision;

		for (unsigned int i = 0; i < 3; i++)
		{
			float v = point[i];

			if (v < aabbWorldMin[i])
				v = aabbWorldMin[i];
			if (v > aabbWorldMax[i])
				v = aabbWorldMax[i];

			pointOfCollision[i] = v;
		}

		pointOfCollision.w = 1.0f;

		return pointOfCollision;
	}

	static glm::vec4 ClosestPointToSphere(glm::vec4 point, SphereCollider* sphere)
	{
		glm::vec4 sphereWorldPosition = sphere->GetWorldPosition();

		// First, get a vetor from the sphere to the point
		glm::vec4 sphereToPoint = point - sphereWorldPosition;
		// Normalize that vector
		sphereToPoint = normalize(sphereToPoint);
		// Adjust it's length to point to edge of sphere
		sphereToPoint *= sphere->GetRadius();
		// Translate into world space
		glm::vec4 worldPoint = sphereWorldPosition + sphereToPoint;

		// Return new point
		return glm::vec4(worldPoint.x, worldPoint.y, worldPoint.z, 1.0f);
	}

	static glm::vec4 ClosestPointToCapsule(glm::vec4 point, CapsuleCollider* cap)
	{
		SphereCollider sphere(ClosestPointOnLine(cap->GetWorldStart(), cap->GetWorldEnd(), point), cap->GetRadius(), cap->GetColliderTag());
		sphere.m_GameObject = cap->m_GameObject;
		return ClosestPointToSphere(point, &sphere);
	}

	static glm::vec4 ClosestPointToCollider(glm::vec4 point, Collider* collider)
	{
		switch (collider->GetColliderType())
		{
			case ColliderType::AABB:
				return ClosestPointToAABB(point, (AABBCollider*)collider);
			case ColliderType::Sphere:
				return ClosestPointToSphere(point, (SphereCollider*)collider);
			case ColliderType::Capsule:
				return ClosestPointToCapsule(point, (CapsuleCollider*)collider);
			default:
				return glm::vec4(0, 0, 0, 0);
		}
	}

	///////////////////////////
	// Mesh Collision Functions
	///////////////////////////

	static int TestSphereTriangle(SphereCollider& s, Triangle tri, glm::vec3 &p)
	{
		// Find point P on triangle ABC closest to sphere center
		p = ClosestPointToTriangle(s.GetPosition(), tri.a, tri.b, tri.c);
		// Sphere and triangle intersect if the (squared) distance from sphere
		// center to point p is less than the (squared) sphere radius
		glm::vec3 v = p - glm::vec3(s.GetPosition());
		return glm::dot(v, v) <= s.GetRadius() * s.GetRadius();
	}

	static int TestCapsuleTriangle(SphereCollider& s1, SphereCollider& s2, Triangle tri, glm::vec3& collisionPoint)
	{
		glm::vec3 c1, c2;
		int collision = TestSphereTriangle(s1, tri, c1) + TestSphereTriangle(s2, tri, c2);

		// TODO: Pick between c1 anc c2



		return collision;
	}

	static float ClosestPtSegmentSegment(glm::vec3 p1, glm::vec3 q1, glm::vec3 p2, glm::vec3 q2, float &s, float &t, glm::vec3 &c1, glm::vec3 &c2)
	{
		glm::vec3 d1 = q1 - p1; // Direction vector of segment S1
		glm::vec3 d2 = q2 - p2; // Direction vector of segment S2
		glm::vec3 r = p1 - p2;
		float a = glm::dot(d1, d1); // Squared length of segment S1, always nonnegative
		float e = glm::dot(d2, d2); // Squared length of segment S2, always nonnegative
		float f = glm::dot(d2, r);
		// Check if either or both segments degenerate into points
		if (a <= FLT_EPSILON && e <= FLT_EPSILON)
		{
			// Both segments degenerate into points
			s = t = 0.0f;
			c1 = p1;
			c2 = p2;
			return glm::dot(c1 - c2, c1 - c2);
		}
		if (a <= FLT_EPSILON)
		{
			// First segment degenerates into a point
			s = 0.0f;
			t = f / e; // s = 0 => t = (b*s + f) / e = f / e
			t = glm::clamp(t, 0.0f, 1.0f);
		}
		else
		{
			float c = glm::dot(d1, r);
			if (e <= FLT_EPSILON)
			{
				// Second segment degenerates into a point
				t = 0.0f;
				s = glm::clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
			}
			else
			{
				// The general nondegenerate case starts here
				float b = glm::dot(d1, d2);
				float denom = a*e - b*b; // Always nonnegative
										 // If segments not parallel, compute closest point on L1 to L2 and
										 // clamp to segment S1. Else pick arbitrary s (here 0)
				if (denom != 0.0f)
				{
					s = glm::clamp((b*f - c*e) / denom, 0.0f, 1.0f);
				}
				else s = 0.0f;
				// Compute point on L2 closest to S1(s) using
				// t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
				t = (b*s + f) / e;
				// If t in [0,1] done. Else clamp t, recompute s for the new value
				// of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
				// and clamp s to [0, 1]
				if (t < 0.0f)
				{
					t = 0.0f;
					s = glm::clamp(-c / a, 0.0f, 1.0f);
				}
				else if (t > 1.0f)
				{
					t = 1.0f;
					s = glm::clamp((b - c) / a, 0.0f, 1.0f);
				}
			}
		}

		c1 = p1 + d1 * s;
		c2 = p2 + d2 * t;
		return dot(c1 - c2, c1 - c2);
	}

	///////////
	// Raycasts
	///////////

	// Raycast to Sphere
	// Returns -1 if no collision
	static int RaycastToSphere(glm::vec4 point, glm::vec4 direction, SphereCollider* sphere)
	{
		glm::vec4 c = sphere->GetWorldPosition();
		float r = sphere->GetRadius();

		glm::vec4 e = c - point;
		// Using Length here would cause floating point error to creep in
		float Esq = length2(e);
		float a = dot(e, direction);
		float b = sqrt(Esq - (a * a));
		float f = sqrt((r * r) - (b * b));

		// No collision
		if (r * r - Esq + a * a < FLT_EPSILON)
		{
			return -1; // -1 is invalid.
		}
		// Ray is inside
		else if (Esq < r * r)
		{
			return (int)(a + f); // Just reverse direction
		}
		// else Normal intersection
		return (int)(a - f);
	}


	//static glm::vec4 RaycastToSphere2(glm::vec4 point, glm::vec4 direction, SphereCollider* sphere)
	//{
	//	direction = normalize(direction);

	//	auto l = point - sphere->GetPosition();
	//	auto tca = dot(l, direction);

	//	auto d2 = dot(l, l) - tca * tca;

	//	auto thc = sqrt(sphere->GetRadius() * sphere->GetRadius() - d2);
	//	auto t = tca - thc;


	//	auto collisionPoint = point + direction * t;
	//	return collisionPoint;
	//}

	// Raycast to AABB
	// Returns -1 if no collision

	static int RaycastToAABB(glm::vec4 point, glm::vec4 direction, AABBCollider* aabb)
	{
		glm::vec3 t1 = (glm::vec3(aabb->GetWorldMin()) - glm::vec3(point));
		glm::vec3 t2 = (glm::vec3(aabb->GetWorldMax()) - glm::vec3(point));
		if (t1.x == 0.0f) t1.x = FLT_EPSILON;
		if (t1.y == 0.0f) t1.y = FLT_EPSILON;
		if (t1.z == 0.0f) t1.z = FLT_EPSILON;
		if (t2.x == 0.0f) t2.x = FLT_EPSILON;
		if (t2.y == 0.0f) t2.y = FLT_EPSILON;
		if (t2.z == 0.0f) t2.z = FLT_EPSILON;
		t1 /= glm::vec3(direction);
		t2 /= glm::vec3(direction);

		float tmin = fmax(fmax(fmin(t1.x, t2.x), fmin(t1.y, t2.y)), fmin(t1.z, t2.z));
		float tmax = fmin(fmin(fmax(t1.x, t2.x), fmax(t1.y, t2.y)), fmax(t1.z, t2.z));

		// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
		if (tmax < 0)
		{
			return -1;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			return -1;
		}

		if (tmin < FLT_EPSILON)
		{
			return 1;
		}

		return 1;
	}

	static glm::vec4 RaycastToAABBPoint(glm::vec4 point, glm::vec4 direction, AABBCollider* aabb)
	{
		glm::vec3 t1 = (glm::vec3(aabb->GetWorldMin()) - glm::vec3(point));
		glm::vec3 t2 = (glm::vec3(aabb->GetWorldMax()) - glm::vec3(point));
		if (t1.x == 0.0f) t1.x = FLT_EPSILON;
		if (t1.y == 0.0f) t1.y = FLT_EPSILON;
		if (t1.z == 0.0f) t1.z = FLT_EPSILON;
		if (t2.x == 0.0f) t2.x = FLT_EPSILON;
		if (t2.y == 0.0f) t2.y = FLT_EPSILON;
		if (t2.z == 0.0f) t2.z = FLT_EPSILON;
		t1 /= glm::vec3(direction);
		t2 /= glm::vec3(direction);

		float tmin = fmax(fmax(fmin(t1.x, t2.x), fmin(t1.y, t2.y)), fmin(t1.z, t2.z));
		float tmax = fmin(fmin(fmax(t1.x, t2.x), fmax(t1.y, t2.y)), fmax(t1.z, t2.z));

		if (tmax < 0)
			return point + glm::normalize(direction) * INFINITY;

		if (tmin > tmax)
			return point + glm::normalize(direction) * INFINITY;

		if (tmin < FLT_EPSILON)
			return point + glm::normalize(direction) * tmax;

		return point + glm::normalize(direction) * tmin;
	}

	// Raycast to Capsule
	// Returns -1 if no collision
	static int RaycastToCapsule(glm::vec4 point, glm::vec4 direction, CapsuleCollider* cap)
	{
		SphereCollider sphere(ClosestPointOnLine(cap->GetWorldStart(), cap->GetWorldEnd(), point), cap->GetRadius(), cap->GetColliderTag());
		sphere.m_GameObject = cap->m_GameObject;
		return RaycastToSphere(point, direction, &sphere);
	}

	// Raycasts against any type of collider that is not a MeshCollider
	static int Raycast(glm::vec4 point, glm::vec4 direction, Collider* collider)
	{
		switch (collider->GetColliderType())
		{
			case ColliderType::AABB:
				return RaycastToAABB(point, direction, (AABBCollider*)collider);
			case ColliderType::Sphere:
				return RaycastToSphere(point, direction, (SphereCollider*)collider);
			case ColliderType::Capsule:
				return RaycastToCapsule(point, direction, (CapsuleCollider*)collider);
			default:
				return -1;
		}
	}

	// Returns all Colliders that the Raycast collides against
	static std::vector<Object*> RaycastAgainstAll(glm::vec4 point, glm::vec4 direction, ObjectManager* manager)
	{
		std::vector<Object*> collidingAgainst;

		auto dynamicObjects = manager->GetDynamicCollidableObjects();
		auto staticObjects = manager->GetStaticCollidableObjects();
		//auto staticObjects = manager->GetStaticObjectsRaycast(point, direction);
		
		for (size_t i = 0; i < dynamicObjects.size(); i++)
		{
			auto colliders = dynamicObjects[i]->GetColliders();
			auto meshColliders = dynamicObjects[i]->GetMeshColliders();
			bool addedThisObject = false;

			for (size_t j = 0; j < colliders.size(); j++)
			{
				if (addedThisObject)
					break;

				if (Raycast(point, direction, colliders[j]) > 0)
				{
					collidingAgainst.push_back(dynamicObjects[i]);
					addedThisObject = true;
				}
			}

			for (size_t j = 0; j < meshColliders.size(); j++)
			{
				if (addedThisObject)
					break;

				if (Raycast(point, direction, &meshColliders[j]->GetBounds()) > 0)
				{
					collidingAgainst.push_back(dynamicObjects[i]);
					addedThisObject = true;
				}
			}
		}

		for (size_t i = 0; i < staticObjects.size(); i++)
		{
			auto colliders = staticObjects[i]->GetColliders();
			auto meshColliders = staticObjects[i]->GetMeshColliders();
			bool addedThisObject = false;

			for (size_t j = 0; j < colliders.size(); j++)
			{
				if (addedThisObject)
					break;

				if (Raycast(point, direction, colliders[j]) > 0)
				{
					collidingAgainst.push_back(staticObjects[i]);
					addedThisObject = true;
				}
			}

			for (size_t j = 0; j < meshColliders.size(); j++)
			{
				if (addedThisObject)
					break;

				if (Raycast(point, direction, &meshColliders[j]->GetBounds()) > 0)
				{
					collidingAgainst.push_back(staticObjects[i]);
					addedThisObject = true;
				}
			}
		}

		return collidingAgainst;
	}

	// Returns the closest collider the Raycast collides against
	static Object* RaycastAgainstClosest(glm::vec4 point, glm::vec4 direction, ObjectManager* manager)
	{
		std::vector<Object*> &dynamicObjects = manager->GetDynamicCollidableObjects();
		std::vector<Object*> &staticObjects = manager->GetStaticObjectsRaycast(point, direction);

		std::vector<Object*> objects;
		objects.resize(dynamicObjects.size() + staticObjects.size());
		memcpy(objects.data(), dynamicObjects.data(), dynamicObjects.size() * sizeof(dynamicObjects[0]));
		memcpy(objects.data() + dynamicObjects.size(), staticObjects.data(), staticObjects.size() * sizeof(staticObjects[0]));

		float dist2 = FLT_MAX;
		Object* object = nullptr;
		// Dynamic Objects
		for (unsigned int i = 0; i < objects.size(); i++)
		{
			auto dynObject = objects[i];

			auto colliders = dynObject->GetColliders();
			auto meshColliders = dynObject->GetMeshColliders();

			// Normal Colliders
			for (size_t j = 0; j < colliders.size(); j++)
			{
				if (Raycast(point, direction, colliders[j]) > 0)
				{
					glm::vec4 closestPoint = ClosestPointToCollider(point, colliders[j]);
					float currDist = length2(closestPoint - point);

					if (currDist < dist2)
					{
						dist2 = currDist;
						object = dynObject;
					}
				}
			}

			// Mesh Colliders
			for (size_t j = 0; j < meshColliders.size(); j++)
			{
				glm::vec4 hit = RaycastToAABBPoint(point, direction, &meshColliders[j]->GetBounds());
				if (length2(hit) != INFINITY)
				{
					float currDist = length2(hit - point);

					if (currDist < dist2)
					{
						dist2 = currDist;
						object = dynObject;
					}
				}
			}
		}

		if(manager == ObjectManager::GetInstance())
			Renderer::GetInstance()->AddLine(point, point + direction * sqrt(dist2));

		return object;
	}

#pragma optimize("", off)
	static Object* RaycastAgainstClosest(glm::vec4 point, glm::vec4 direction, ObjectManager* manager, glm::vec4& finalHit)
	{
		std::vector<Object*> &dynamicObjects = manager->GetDynamicCollidableObjects();
		std::vector<Object*> &staticObjects = manager->GetStaticObjectsRaycast(point, direction);

		std::vector<Object*> objects;
		objects.resize(dynamicObjects.size() + staticObjects.size());
		memcpy(objects.data(), dynamicObjects.data(), dynamicObjects.size() * sizeof(dynamicObjects[0]));
		memcpy(objects.data() + dynamicObjects.size(), staticObjects.data(), staticObjects.size() * sizeof(staticObjects[0]));

		float dist2 = FLT_MAX;
		Object* object = nullptr;
		// Dynamic Objects
		for (unsigned int i = 0; i < objects.size(); i++)
		{
			auto dynObject = objects[i];

			auto meshColliders = dynObject->GetMeshColliders();

			// Mesh Colliders
			for (size_t j = 0; j < meshColliders.size(); j++)
			{
				glm::vec4 hit = RaycastToAABBPoint(point, direction, &meshColliders[j]->GetBounds());
				if (length2(hit) != INFINITY)
				{
					float currDist = length2(hit - point);

					if (currDist < dist2)
					{
						dist2 = currDist;
						object = dynObject;
						finalHit = hit;
					}
				}
			}
		}

		//if(manager == ObjectManager::GetInstance())
		//	Renderer::GetInstance()->AddLine(point, point + direction * sqrt(dist2));

		return object;
	}
#pragma optimize("", on)
	static Object* RaycastAgainstClosest(glm::vec4 point, glm::vec4 direction, std::vector<Object*> objects)
	{
		float dist2 = FLT_MAX;
		Object* object = nullptr;
		// Dynamic Objects
		for (unsigned int i = 0; i < objects.size(); i++)
		{
			auto dynObject = objects[i];

			auto colliders = dynObject->GetColliders();
			auto meshColliders = dynObject->GetMeshColliders();

			// Normal Colliders
			for (size_t j = 0; j < colliders.size(); j++)
			{
				if (Raycast(point, direction, colliders[j]) > 0)
				{
					glm::vec4 closestPoint = ClosestPointToCollider(point, colliders[j]);
					float currDist = length2(closestPoint - point);

					if (currDist < dist2)
					{
						dist2 = currDist;
						object = dynObject;
					}
				}
			}

			// Mesh Colliders
			for (size_t j = 0; j < meshColliders.size(); j++)
			{
				if (Raycast(point, direction, &meshColliders[j]->GetBounds()) > 0)
				{
					glm::vec4 closestPoint = ClosestPointToCollider(point, &meshColliders[j]->GetBounds());
					float currDist = length2(closestPoint - point);

					if (currDist < dist2)
					{
						dist2 = currDist;
						object = dynObject;
					}
				}
			}
		}

		return object;
	}

	/////////////////////
	// Collider Functions
	/////////////////////

	// AABB to AABB
	static bool AABB_to_AABB(AABBCollider& lhs, AABBCollider& rhs, glm::vec4 &pointOfCollision)
	{
		for (unsigned int i = 0; i < 3; i++)
			if (lhs.GetWorldMax()[i] < rhs.GetWorldMin()[i] || lhs.GetWorldMin()[i] > rhs.GetWorldMax()[i])
				return false;

		return true;
	}

	static bool AABB_to_Sphere(AABBCollider& aabb, SphereCollider& sphere, glm::vec4 &pointOfCollision)
	{
		glm::vec4 sphereWorldPosition = sphere.GetWorldPosition();

		glm::vec4 closestPoint = ClosestPointToAABB(sphereWorldPosition, &aabb);
		glm::vec4 differenceVec = sphereWorldPosition - closestPoint;

		float distSquared = length2(differenceVec);
		float radiusSquared = sphere.GetRadius() * sphere.GetRadius();

		return distSquared < radiusSquared;
	}

	// AABB to Capsule
	static bool AABB_to_Capsule(AABBCollider& aabb, CapsuleCollider& capsule, glm::vec4 &pointOfCollision)
	{
		SphereCollider s1(capsule.GetStart(), capsule.GetRadius(), capsule.GetColliderTag()), s2(capsule.GetEnd(), capsule.GetRadius(), capsule.GetColliderTag());
		s1.m_GameObject = capsule.m_GameObject;
		s2.m_GameObject = capsule.m_GameObject;

		int collision = AABB_to_Sphere(aabb, s1, pointOfCollision) + AABB_to_Sphere(aabb, s2, pointOfCollision);
		return collision;
	}

	// Capsule to Capsule
	static bool Capsule_to_Capsule(CapsuleCollider& capsule1, CapsuleCollider& capsule2, glm::vec4 &pointOfCollision)
	{
		// Compute (squared) distance between the inner structures of the capsules
		float s, t;
		glm::vec3 c1, c2;
		float dist2 = ClosestPtSegmentSegment(capsule1.GetWorldStart(), capsule1.GetWorldEnd(), capsule2.GetWorldStart(), capsule2.GetWorldEnd(), s, t, c1, c2);
		// If (squared) distance smaller than (squared) sum of radii, they collide
		float radius = capsule1.GetRadius() + capsule2.GetRadius();
		return dist2 <= radius * radius;
	}

	// Capsule to Sphere
	static bool Capsule_to_Sphere(CapsuleCollider& capsule, SphereCollider& sphere, glm::vec4 &pointOfCollision)
	{
		glm::vec4 capWorldStart = capsule.GetWorldStart();
		glm::vec4 capWorldEnd = capsule.GetWorldEnd();
		glm::vec4 sphereWorldPosition = sphere.GetWorldPosition();

		glm::vec4 BA = capWorldEnd - capWorldStart;
		glm::vec4 CA = sphereWorldPosition - capWorldStart;

		BA.w = 1;
		CA.w = 1;

		float d = (dot(CA, BA) / dot(BA, BA));

		if (d > 1)
			d = 1;
		else if (d < 0)
			d = 0;

		glm::vec4 whomstve = capWorldStart + BA * d;

		//float dist = (whomstve.x - sphereWorldPosition.x) * (whomstve.x - sphereWorldPosition.x) +
		//			 (whomstve.y - sphereWorldPosition.y) * (whomstve.y - sphereWorldPosition.y) +
		//			 (whomstve.z - sphereWorldPosition.z) * (whomstve.z - sphereWorldPosition.z);

		float dist = length2(glm::vec3(whomstve) - glm::vec3(sphereWorldPosition));

		float sumRadii = sphere.GetRadius() + capsule.GetRadius();
		sumRadii *= sumRadii;

		if (dist <= sumRadii)
			return true;

		return false;
	}

	// Sphere to Sphere
	static bool Sphere_to_Sphere(SphereCollider& lhs, SphereCollider& rhs, glm::vec4 &pointOfCollision, glm::vec4& nCollisionVector, float& depth)
	{
		glm::vec4 lhsWorldPosition = lhs.GetWorldPosition();
		glm::vec4 rhsWorldPosition = rhs.GetWorldPosition();

		float dist = length2(rhsWorldPosition - lhsWorldPosition);
		float sumRadii = lhs.GetRadius() + rhs.GetRadius();
		sumRadii *= sumRadii;

		if (dist > sumRadii)
			return false;

		if (dist == 0.0f)
		{
			lhsWorldPosition += FLT_EPSILON;
		}

		pointOfCollision = ClosestPointToSphere(lhsWorldPosition, &rhs);
		nCollisionVector = normalize(lhsWorldPosition - pointOfCollision);
		depth = -length(lhsWorldPosition - rhsWorldPosition) + (lhs.GetRadius() + rhs.GetRadius());

		if (dist <= sumRadii)
			return true;

		return false;
	}

	//////////////////////////////
	// Generic Collision Functions
	//////////////////////////////
	static bool Collide(AABBCollider* lhs, AABBCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return AABB_to_AABB(*lhs, *rhs, pointOfCollision);
	}

	static bool Collide(AABBCollider* lhs, SphereCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return AABB_to_Sphere(*lhs, *rhs, pointOfCollision);
	}

	static bool Collide(AABBCollider* lhs, CapsuleCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return AABB_to_Capsule(*lhs, *rhs, pointOfCollision);
	}

	static bool Collide(CapsuleCollider* lhs, AABBCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return AABB_to_Capsule(*rhs, *lhs, pointOfCollision);
	}

	static bool Collide(CapsuleCollider* lhs, SphereCollider* rhs, glm::vec4 &pointOfCollision, glm::vec4& nCollisionVector, float& depth)
	{
		return Capsule_to_Sphere(*lhs, *rhs, pointOfCollision);
	}

	static bool Collide(CapsuleCollider* lhs, CapsuleCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return Capsule_to_Capsule(*lhs, *rhs, pointOfCollision);
	}

	static bool Collide(SphereCollider* lhs, AABBCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return AABB_to_Sphere(*rhs, *lhs, pointOfCollision);
	}

	static bool Collide(SphereCollider* lhs, SphereCollider* rhs, glm::vec4 &pointOfCollision, glm::vec4& nCollisionVector, float& depth)
	{
		return Sphere_to_Sphere(*lhs, *rhs, pointOfCollision, nCollisionVector, depth);
	}

	static bool Collide(SphereCollider* lhs, CapsuleCollider* rhs, glm::vec4 &pointOfCollision)
	{
		return Capsule_to_Sphere(*rhs, *lhs, pointOfCollision);
	}
}

namespace CollisionResponse
{
	static void PushOut_SphereToSphere(CollisionEvent e)
	{
		if ((e.m_CollidingWith->GetColliderType() == ColliderType::AABB || e.m_CollidingWith->GetColliderType() == ColliderType::Mesh) || e.m_CollidingWith->GetIsTrigger() == true || e.m_ThisObject->m_GameObject->m_Static)
			return;

		e.m_ThisObject->m_GameObject->m_Transform[3] += (e.m_nCollisionVector * (e.m_CollisionDepth + FLT_EPSILON));
	}

	static void PushOut_Capsule_to_Capsule(CollisionEvent e)
	{
		if (e.m_CollidingWith->GetColliderType() != ColliderType::Capsule || e.m_CollidingWith->GetIsTrigger() == true || e.m_ThisObject->m_GameObject->m_Static)
			return;


	}

	static void PushOut_SphereToMesh(CollisionEvent e)
	{
		if (e.m_CollidingWith->GetColliderType() != ColliderType::Mesh)
			return;

		RigidBodyComponent* body = e.m_ThisObject->m_GameObject->GetComponent<RigidBodyComponent>();
		glm::vec3 velocity = (body->GetVelocity());
		glm::vec3 rflect = 1.0f * e.m_FaceNormal * glm::dot(e.m_FaceNormal, (body->GetVelocity()));

		body->AddVelocity(-rflect);
		e.m_ThisObject->m_GameObject->m_Transform[3] += (e.m_nCollisionVector * (e.m_CollisionDepth + FLT_EPSILON));
	}
}