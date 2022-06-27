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

glm::vec3 Light::illuminate(const Ray &r, const HitRecord &hr) const
{
	glm::vec3 x = r.evaluate(hr.t); // Point on surface that was hit
	glm::vec3 l = glm::normalize(position - x); // light direction
	glm::vec3 n = hr.n; //surface normal, we assume this to be normalized already

	// irradiance, describing the amount of radiant power per unit area.
	// This computation of irradiance is specific for point light sources
	
	// The point light is too far. Maybe I should not use glm::length2(position - x).
	// This factor is making my irradiance E extremely small in length
	glm::vec3 E = colour * (std::fmax(0, glm::dot(n,l)) / glm::length2(position - x));

	// view direction from the surface point
	glm::vec3 v = -glm::normalize(r.d); 

	// The bidirectional reflectance distrubution function, or BRDF describes how the
	// reflectance varies as functions of l and v. 
	glm::vec3 k = hr.p->m_material->evaluate(l,v,n); 

	// The amount of reflected light is computed seperately for
	// the red, green, and blue channels.
	return {k.r * E.r, k.g * E.g, k.b * E.b};
}
