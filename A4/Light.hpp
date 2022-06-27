#pragma once

#include <iosfwd>
#include "Primitive.hpp"
#include "Ray.hpp"

#include <glm/glm.hpp>

// Represents a simple point light.
struct Light {
  Light();
  
  glm::vec3 colour; // Intensity of source ? 
  glm::vec3 position;
  double falloff[3];

  // The ray r hits the surface in hr at time hr.t. We return the amount of
  // reflected light of the surface at the intersection point. Note that this
  // computation is carried out seperately for the red, green, and blue channels,
  // so we return a vec3
  virtual glm::vec3 illuminate(const Ray &r, const HitRecord &hr) const;
};

std::ostream& operator<<(std::ostream& out, const Light& l);
