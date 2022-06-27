#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
	, kd(m_kd)
	, ks(m_ks)
{}

PhongMaterial::~PhongMaterial()
{}

glm::vec3 PhongMaterial::evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const 
{
	glm::vec3 h = glm::normalize(l+v);

	glm::vec3 reflectance = m_kd;

	float pi_inv = (float) 1/M_PI;

	return reflectance*pi_inv + m_ks * (float) glm::pow( std::fmax(0, glm::dot(n,h)), m_shininess );
}
