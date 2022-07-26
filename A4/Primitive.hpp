#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <list>
#include <string>
#include "Material.hpp"
#include "Ray.hpp"
#define RAY_DISTANCE_MAX 2000

class Primitive; 

struct SurfaceParams
{
	glm::mat4 trans;
	glm::mat4 inv_trans;
	Material *material;
	std::string name;
	// May also put something like texture ?
};

struct HitRecord 
{
	const Primitive *p; // Primitive that was hit
	const SurfaceParams *params; // Paramters of the surface (material, transfomrations, etc)
	float t; // parameter of the ray where the intersection happened
	bool miss; // Whether the ray actually hit the surface or not
	glm::vec3 n; // Normal of surface at the intersection point
	glm::vec3 hit_point; // point on the surface that was hit 

	// Texture coordinates
	double u;
	double v;

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
	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1, const SurfaceParams &sp = SurfaceParams()) const;

	// Hit the untransformed object
	virtual void hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const;

	virtual void updateTextureCoordinates(HitRecord &hr) const;

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

// For the nonhierarchical shapes, we will assume they are untransformed.
class NonhierSphere : public Primitive {
public:
	NonhierSphere(const glm::vec3& pos, double radius);
	virtual ~NonhierSphere();

	virtual void hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;

	const glm::vec3 &pos;
	const double &r;

private:
	glm::vec3 m_pos;
	double m_radius;
};

class NonhierBox : public Primitive {
public:
	NonhierBox(const glm::vec3& pos, double size);
	NonhierBox(const glm::vec3 &bmin, const glm::vec3 &bmax);

	virtual void hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;

	const glm::vec3 &bmin() const;
	const glm::vec3 &bmax() const;

	virtual ~NonhierBox();

private:

	// Given a point on the box, return the surface normal
	glm::vec3 computeNormal(const glm::vec3 &) const;

	glm::vec3 m_min;
	glm::vec3 m_max;
};

// Convenience class for determining how a ray hits a group of surfaces...
struct SurfaceGroup : public Primitive 
{
	SurfaceGroup(const std::list<Primitive *> & surfaces = {}, const std::list<SurfaceParams> &params = {});

	virtual void hit(HitRecord &hr, const Ray &r, float t_0, float t_1, const SurfaceParams &sp = SurfaceParams()) const override;
	virtual void hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const override;

	// Maybe we should also store material? And name? Perhaps just keep another bookkeeping class called PrimitiveParameters or something
	std::list<Primitive *> m_surfaces; 
	std::list<SurfaceParams> m_surfaceParams;
};
