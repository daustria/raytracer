#include <iostream>
#include "Texture.hpp"


// Implementation of Image Texture is mostly copied from 'Ray Tracing The Next Week'

ImageTexture::ImageTexture() : m_data(nullptr), m_width(0), m_height(0), bytes_per_scanline(0)
{

}


ImageTexture::ImageTexture(const char *filename)
{
	int components_per_pixel = bytes_per_pixel;

	m_data = stbi_load(filename, &m_width, &m_height, &components_per_pixel, components_per_pixel);

	if (!m_data) {
		std::cerr << " Could not load texture image file " << filename << std::endl;
		m_width = 0;
		m_height = 0;
	}

	bytes_per_scanline = bytes_per_pixel * m_width;

}

ImageTexture::~ImageTexture()
{
	delete m_data;
}

glm::vec3 ImageTexture::value(double u, double v, const glm::vec3 &p) const 
{
	if (m_data == nullptr) {
		// Return cyan as a debugging aid
		return {0, 1.0f, 1.0f};
	}

	// Clamp to [0,1]x[0,1]
	u = glm::clamp(u, 0.0, 1.0);
	v = 1.0 - glm::clamp(v, 0.0, 1.0); // We 'flip' the v part

	// Now sample the image with texture coordinates
	int i = static_cast<int>(u*m_width);
	int j = static_cast<int>(v*m_height);


	// Clamp the integers, since it should be less than 1.0
	if (i >= m_width) {
		i = m_width - 1;
	}	

	if (j >= m_height) {
		j = m_height - 1;
	}

	unsigned char *pixel = m_data + j*bytes_per_scanline + i*bytes_per_pixel;
	glm::vec3 colour{pixel[0], pixel[1], pixel[2]};
	colour = (1/255.0f) * colour;
	return colour;
}


