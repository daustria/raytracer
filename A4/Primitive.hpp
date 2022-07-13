#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <list>
#include "Material.hpp"
#include "Ray.hpp"

class Primitive; 

struct HitRecord 
{
	const Primitive *p; // surface that was hit
	float t; // parameter of the ray where the intersection happened
	bool miss; // Whether the ray actually hit the surface or not
	glm::vec3 n; // Normal of surface at the intersection point
	glm::vec3 hit_point; // point on the surface that was hit 

	// We may need more things like texture coordinates, later on
};

enum class PrimitiveType 
{
	None,
	Sphere,
	Cube,
	NH_Sphere,
	NH_Box,
	Mesh,
	Group
};

class Primitive {
public:
	Primitive();
	virtual ~Primitive();
	
	// Fill in the HitRecord hr with data about whether the ray r intersected this primitive
	// in the interval (t_0, t_1)
	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const;

	PrimitiveType m_primitiveType;	

	Material *m_material; 
	glm::mat4 m_transform; // What the primitive is transformed by.. but really we are going to 
	// use this to transform the ray 
	friend std::ostream & operator << (std::ostream &, const Primitive &);

};

void transformVertex(glm::vec3 &vertex, const glm::mat4 &transform);

class Sphere : public Primitive {
public:
	virtual ~Sphere();
};

class Cube : public Primitive {
public:
	virtual ~Cube();
};

// For the nonhierarchical shapes, we will assume they are untransformed.
class NonhierSphere : public Primitive {
public:
	NonhierSphere(const glm::vec3& pos, double radius);
	virtual ~NonhierSphere();

	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;

	const glm::vec3 &pos;
	const double &r;

private:
	glm::vec3 m_pos;
	double m_radius;
};

class NonhierBox : public Primitive {
public:
	NonhierBox(const glm::vec3& pos, double size);

	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;


	const glm::vec3 &bmin() const;
	const glm::vec3 &bmax() const;

	virtual ~NonhierBox();

private:

	// Given a point on the box, return the surface normal
	glm::vec3 computeNormal(const glm::vec3 &) const;

	glm::vec3 m_pos;
	double m_size;
	glm::vec3 m_min;
	glm::vec3 m_max;
};

// Convenience class for determining how a ray hits a group of surfaces...
struct SurfaceGroup : public Primitive 
{
	SurfaceGroup(const std::list<Primitive *> & surfaces = {});
	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const;
	std::list<Primitive *> m_surfaces; 
};
