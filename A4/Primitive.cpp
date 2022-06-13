#include "Primitive.hpp"

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

	float discriminant = b*b - 4*a*c;

	if (discriminant < 0) {
		// No solutions
		hr.miss = true;
	} else {
		// The discriminant is non-zero so there is at least one solution.

		float sqrt_discriminant = sqrt(discriminant);

		bool outside_interval = false;
		float t_intersection = 0;

		//Discriminant close to 0, we'll say here we have one solution
		if ( abs(discriminant) < epsilon ) {
			// The quadratic formula has two choices, we'll take the 
			// positive one for this case

			t_intersection = (-b + sqrt_discriminant) / 2*a;

			if (t_intersection > t_0 && t_intersection < t_1) {
				// Do nothing
			} else {
				outside_interval = true;
			}

		} else {
			// Discriminant is non-zero so the ray intersects the sphere twice: One where it enters
			// and one where it leaves. We must choose the 'closer' one.
			float soln_1 = (-b + sqrt_discriminant)/ 2*a;
			float soln_2 = (-b - sqrt_discriminant) / 2*a;

			// Find which of the solutions are smaller, and which is larger
			float smaller = std::fmin(soln_1, soln_2);
			float larger = std::fmax(soln_1, soln_2);

			// If the smaller of the two is in the interval (t_0, t_1), then it is the one we take.
			// Otherwise if the larger is in the interval, we take that one. If none of them are in the interval
			// then it counts as a miss.
			if ( smaller > t_0 && smaller < t_1 ) {
				t_intersection = smaller;
			} else if ( larger > t_0 && larger < t_1 ) {
				t_intersection = larger;
			} else {
				outside_interval = true;
			}

		}

		// All that is left to do is to update the hit record

		if (outside_interval) {
			hr.miss = true;
		} else {
			hr.miss = false;
			hr.t = t_intersection;
			
			glm::vec3 intersection_point = r.evaluate(t_intersection);
			glm::vec3 sphere_normal = (intersection_point - m_pos) / (float) m_radius;
			hr.n = sphere_normal;
		}
	}

}

NonhierBox::~NonhierBox()
{
}
