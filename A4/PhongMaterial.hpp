#pragma once

#include <glm/glm.hpp>
#include <string>
#include "Material.hpp"
#include "Ray.hpp"

class PhongMaterial : public Material 
{
public:
	PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess);
	PhongMaterial(const std::string &fname);

	virtual ~PhongMaterial();
	virtual glm::vec3 evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const override;
	virtual void sampleTexture(double u, double v) override;

private:
	glm::vec3 m_ka;			// Ambient Color
	glm::vec3 m_kd;			// Diffuse Color
	glm::vec3 m_ks;			// Specular Color

	double m_ns; 			// Specular Exponent
	std::string map_Ka; 		// Ambient Texture Map
	std::string map_Kd; 		// Diffuse Texture Map
	std::string map_Ks; 		// Specular Texture Map

	// Texture coordinates to use on evaluate
	glm::vec3 m_textureColour;

public:
	const glm::vec3 &kd;
	const glm::vec3 &ks;

};
