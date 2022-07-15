#ifndef RAY_H
#define RAY_H
#include <glm/glm.hpp>
// Keep this class simple for now.. we may construct ray's in a more pedantic fashion later
class Ray
{
public:
	// Custom constructor, just want to check that the direction is not zero
	Ray(const glm::vec3 &origin, const glm::vec3 &direction);

	glm::vec3 evaluate(float t) const;

	// Transforms the ray by left multiplying m
	void transform(const glm::mat4 &m);

private:
	glm::vec3 m_origin;
	glm::vec3 m_direction;
public:
	// Easy access references
	const glm::vec3 &o; // origin
	const glm::vec3 &d; // direction
};
#endif // RAY_H
