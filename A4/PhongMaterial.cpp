#include "PhongMaterial.hpp"
#include <iostream>
#include <fstream>

PhongMaterial::PhongMaterial(
		const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_ns(shininess)
	, kd(m_kd)
	  , ks(m_ks)
{}

std::string getAssetFilePath(const std::string &fname) 
{

	// Only put the assets part if the filename does not already include it 
	std::string assets_dir{"Assets"};
	if (fname.length() > assets_dir.length()) {
		if (fname.substr(0,assets_dir.length()) == assets_dir) {
			return fname;
		}
	}

	return "Assets/"+fname;
}

PhongMaterial::PhongMaterial(const std::string &fname) : kd(m_kd), ks(m_ks)
{

	// Load the material parameters from the file. Note that we only
	// read one material.

	if (fname.substr(fname.size() - 4, fname.size()) != ".mtl") {
		std::cerr << "ERROR in reading file " << fname << std::endl;
		return;
	}

	std::ifstream ifs( getAssetFilePath(fname).c_str() );

	bool loaded_material = false;
	std::string code;
	std::string curline;

	while ( ifs >> code )
	{
		// new material and material name
		if (code == "newmtl")
		{
			if (loaded_material) {
				// We already loaded a material, so bail out
				break;
			} else {
				loaded_material = true;
				std::getline(ifs, curline);
				continue;
			}
		} else if (code == "Ka") {
			// Ambient Color
			float ka_r, ka_g, ka_b;

			ifs >> ka_r >> ka_g >> ka_b;

			m_ka.r = ka_r;
			m_ka.g = ka_g;
			m_ka.b = ka_b;
		} else if (code == "Kd") {
			// Diffuse Colour 
			float kd_r, kd_g, kd_b;

			ifs >> kd_r >> kd_g >> kd_b;

			m_kd.r = kd_r;
			m_kd.g = kd_g;
			m_kd.b = kd_b;
		} else if (code == "Ks") {
			// Specular Colour
			float ks_r, ks_g, ks_b;

			ifs >> ks_r >> ks_g >> ks_b;

			m_ks.r = ks_r;
			m_ks.g = ks_g;
			m_ks.b = ks_b;

		} else if (code == "Ns") {
			// Specular exponent
			ifs >> m_ns;
		} else if (code == "map_Ka") {
			ifs >> map_Ka;
		} else if (code == "map_Kd") {
			ifs >> map_Kd;
			m_texture = new ImageTexture(getAssetFilePath(map_Kd));
			printf("%s | Loading texture %s\n", __func__, map_Kd.c_str());

		} else if (code == "map_Ks") {
			ifs >> map_Ks;
		} else {
			printf("%s | ignoring code %s when reading file %s \n", __func__, code.c_str(), fname.c_str());
			std::getline(ifs, curline);
			continue;
		}

	}
}

PhongMaterial::~PhongMaterial()
{}

void PhongMaterial::sampleTexture(double u, double v)
{
	// Read the texture file and change the parameters of this material accordingly.
	if (m_texture) {
		m_textureColour = m_texture->value(u, v, {0,0,0});
	}
}

glm::vec3 PhongMaterial::evaluate(const glm::vec3 &l, const glm::vec3 &v, const glm::vec3 &n) const 
{	
	glm::vec3 h = glm::normalize(l+v);

	float pi_inv = (float) 1/M_PI;

	glm::vec3 colour = kd*pi_inv + m_ks * (float) glm::pow( std::fmax(0, glm::dot(n,h)), m_ns );

	if (m_texture) {
		// return m_textureColour;
		return {colour.r * m_textureColour.r, colour.g * m_textureColour.g, colour.b * m_textureColour.b};
	} else {
		return colour;
	}
}

