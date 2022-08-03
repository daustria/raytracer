#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include <glm/glm.hpp>
#include <string>

class Texture
{
public:
	virtual glm::vec3 value(double u, double v, const glm::vec3 &p) const = 0;
};


class ImageTexture : public Texture
{
public:
	const static int bytes_per_pixel = 3;

	ImageTexture();

	ImageTexture(const std::string &filename);

	~ImageTexture();


	// The colour of position (u,v) of the texture image
	virtual glm::vec3 value(double u, double v, const glm::vec3& p) const override;

private:
	unsigned char *m_data;
	int m_width;
	int m_height;
	int bytes_per_scanline;
};

#endif // _TEXTURE_H_
