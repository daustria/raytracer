#pragma once

#include <glm/glm.hpp>
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

class Primitive {
public:
	virtual ~Primitive();
	
	// Fill in the HitRecord hr with data about whether the ray r intersected this primitive
	// in the interval (t_0, t_1)
	virtual void hit(HitRecord &hr, Ray r, float t_0, float t_1) const;
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
	NonhierSphere(const glm::vec3& pos, double radius)
		: m_pos(pos), m_radius(radius)
	{
	}
	virtual ~NonhierSphere();

	virtual void hit(HitRecord &hr, Ray r, float t_0, float t_1) const override;

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
