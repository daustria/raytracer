#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess);
  virtual ~PhongMaterial();
  virtual glm::vec3 evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const override;

private:
  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;

public:
  const glm::vec3 &kd;
  const glm::vec3 &ks;

};
