#pragma once

#include <glm/glm.hpp>
#include "Texture.hpp"

class Material {
public:
	virtual ~Material();

	virtual glm::vec3 evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const;
	virtual void sampleTexture(double u, double v);

	Texture *m_texture;

protected:
	Material();
};
