#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <list>
#include "Ray.hpp"

class Primitive; 

struct HitRecord 
{
	const Primitive *p; // surface that was hit
	float t; // Point along the ray where the intersection happened
	bool miss; // Whether the ray actually hit the surface or not
	glm::vec3 n; // Normal of surface at the intersection point

	// We may need more things like texture coordinates, later on
};

enum class PrimitiveType 
{
	None,
	Sphere,
	Cube,
	NH_Sphere,
	NH_Box,
	Group
};

class Primitive {
public:
	Primitive();
	virtual ~Primitive();
	
	// Fill in the HitRecord hr with data about whether the ray r intersected this primitive
	// in the interval (t_0, t_1)
	virtual void hit(HitRecord &hr, Ray r, float t_0, float t_1) const;

	PrimitiveType m_primitiveType;

	friend std::ostream & operator << (std::ostream &, const Primitive &);
};

class Sphere : public Primitive {
public:
	virtual ~Sphere();
};

class Cube : public Primitive {
public:
	virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
	NonhierSphere(const glm::vec3& pos, double radius);
	virtual ~NonhierSphere();

	virtual void hit(HitRecord &hr, Ray r, float t_0, float t_1) const override;

	const glm::vec3 &pos;
	const double &r;

private:
	glm::vec3 m_pos;
	double m_radius;
};

class NonhierBox : public Primitive {
public:
	NonhierBox(const glm::vec3& pos, double size)
		: m_pos(pos), m_size(size)
	{
	}

	virtual ~NonhierBox();

private:
	glm::vec3 m_pos;
	double m_size;
};

// Convenience class for determining how a ray hits a group of surfaces...
struct SurfaceGroup : public Primitive 
{
	SurfaceGroup(const std::list<const Primitive *> & surfaces = {});
	virtual void hit(HitRecord &hr, Ray r, float t_0, float t_1) const;
	std::list<const Primitive *> m_surfaces; 
};
