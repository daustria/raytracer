#include "Primitive.hpp"
#include "polyroots.hpp"

static const float epsilon = 0.01f;

Primitive::~Primitive()
{
}

void Primitive::hit(HitRecord &hr, Ray r, float t_0, float t_1) const
{
	// Default behaviour is just to return a miss
	hr.p = this;
	hr.miss = true;
	return;
}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

// Return true if t is in the interval (t_0, t_1)
bool in_interval(float t, float t_0, float t_1)
{
	return t > t_0 && t < t_1;
}

void NonhierSphere::hit(HitRecord &hr, Ray r, float t_0, float t_1) const
{
	// Get the quadratic equation whose solutions corresponding to where the 
	// ray intersects the sphere.
	
	hr.p = this;
	
	// Precompute some terms that come up often in the equation	
	glm::vec3 offset = r.o - m_pos;

	// The equation is then At^2 + Bt + C, where..
	
	float a = glm::dot(r.d, r.d);
	float b = 2*glm::dot(r.d, offset);
	float c = glm::dot(offset, offset) - m_radius * m_radius;

	// Use the provided code to solve the quadratic equation
	
	double roots[2];	

	size_t num_solns = quadraticRoots(a, b, c, roots);
	float t;

	switch(num_solns)
	{
		case 0:
			hr.miss = true;
			break;
		case 1:
			t = roots[1];
			if (!(t > t_0 && t < t_1)) {
				// The solution is outside the interval so we don't accept it
				hr.miss = true;
			}

			break;
		case 2:
		{
			float smaller = std::fmin(roots[0], roots[1]);
			float larger = std::fmax(roots[0], roots[1]);

			// If the smaller of the two is in the interval (t_0, t_1), then it is the one we take.
			// Otherwise if the larger is in the interval, we take that one. If none of them are in the interval
			// then it counts as a miss.
			if ( smaller > t_0 && smaller < t_1 ) {
				t = smaller;
			} else if ( larger > t_0 && larger < t_1 ) {
				t = larger;
			} else {
				hr.miss = true;
			}

			break;
		}
		default:
			// We shouldn't be here
			abort();

	}

	if (!hr.miss) {
		hr.t = t;	
		glm::vec3 intersection_point = r.evaluate(t);
		glm::vec3 sphere_normal = (intersection_point - m_pos) / (float) m_radius;
		hr.n = sphere_normal;
	}
}

NonhierBox::~NonhierBox()
{
}
