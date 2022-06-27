#include "Material.hpp"

Material::Material()
{}

glm::vec3 Material::evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const
{
	// Default behaviour
	return {0,0,0};
}

Material::~Material()
{}
