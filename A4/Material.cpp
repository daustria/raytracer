#include "Material.hpp"

Material::Material() : m_texture(0)
{}

glm::vec3 Material::evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const
{
	// Default behaviour
	return {0,0,0};
}

void Material::sampleTexture(double u, double v)
{
	// Do nothing as default behaviour

}

Material::~Material()
{
	if (m_texture) {
		delete m_texture;
	}
}
