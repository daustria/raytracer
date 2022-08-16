#include <iostream>

#include <glm/ext.hpp>

#include "Light.hpp"

Light::Light()
  : colour(0.0, 0.0, 0.0),
    position(0.0, 0.0, 0.0)
{
  falloff[0] = 1.0;
  falloff[1] = 0.0;
  falloff[2] = 0.0;
}

std::ostream& operator<<(std::ostream& out, const Light& l)
{
  out << "L[" << glm::to_string(l.colour) 
  	  << ", " << glm::to_string(l.position) << ", ";
  for (int i = 0; i < 3; i++) {
    if (i > 0) out << ", ";
    out << l.falloff[i];
  }
  out << "]";
  return out;
}

glm::vec3 Light::illuminate(const Ray &r, const HitRecord &hr, const SurfaceGroup &surfaces) const
{
	static const float SHADOW_EPSILON(0.01f);

	glm::vec3 x = hr.hit_point;
	glm::vec3 l = glm::normalize(position - x); // light direction

	float dist = glm::length(position - x);

	// First we check if the shading point is in the shadow
	Ray r_shadow(x, l);
	HitRecord hr_shadow;

	surfaces.hit(hr_shadow, r_shadow, SHADOW_EPSILON, dist);

	if (!hr_shadow.miss) {
		// for debugging purposes, i can breakpoint here, seeing what the code looks like
		// when the ray intersects an object
		// surfaces.hit(hr_shadow, r_shadow, SHADOW_EPSILON, RAY_DISTANCE_MAX);

		return {0,0,0};	
	}

	const glm::vec3 &n = hr.n ; //surface normal, we assume this to be normalized already

	// irradiance, describing the amount of radiant power per unit area.
	// This computation of irradiance is specific for point light sources
	
	// We borrow this quadratic attenuation computation from the CS488 course notes
	float attenuation = falloff[0] + falloff[1]*dist + falloff[2]*dist*dist;
	glm::vec3 E = colour * ( std::fmax(0, glm::dot(n,l) / attenuation ));

	// Before we use the material, update it with the texture coordinates. Note that
	// this does nothing if our material does not have a texture with it.
	hr.params->material->sampleTexture(hr.u, hr.v);

	// view direction from the surface point
	glm::vec3 v = -glm::normalize(r.d); 

	// The bidirectional reflectance distrubution function, or BRDF describes how the
	// reflectance varies as functions of l and v. 
	glm::vec3 k = hr.params->material->evaluate(l,v,n);

	glm::vec3 colour_final = {k.r*E.r, k.g*E.g, k.b*E.b};

	return colour_final;
}

