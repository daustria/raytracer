#include "Ray.hpp"
#include <glm/ext.hpp>

Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction) 
	: m_origin(origin), m_direction(direction), o(m_origin), d(m_direction)
{
	static const float EPSILON_RAY = 0.1f;
	// Want to make sure our direction is non-zero. I don't want to 
	// worry about rays with a null direction
	assert(glm::length2(direction) > EPSILON_RAY^2);
}

glm::vec3 Ray::evaluate(float t) const
{
	return o + t*d;
}

void Ray::transform(const glm::mat4 &m)
{
	// First transform the origin..
	glm::vec4 o_{o, 1.0f};

	o_ = m*o_;

	if (o_.w == 1.0f) {
		m_origin = glm::vec3{o_.x, o_.y, o_.z};
	} else {
		// this division shouldnt actually be necessary
		m_origin = glm::vec3{o_.x, o_.y, o_.z} * (1 / (o_.w));
	}

	m_direction = glm::mat3(m)*d;
}
