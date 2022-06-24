#include "Primitive.hpp"
#include "polyroots.hpp"

static const float epsilon = 0.01f;

Primitive::Primitive() : m_primitiveType(PrimitiveType::None), m_material(nullptr)
{

}
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

// Nonhier Sphere --------------------------------------------------------------------

NonhierSphere::NonhierSphere(const glm::vec3& pos, double radius)
	: m_pos(pos), m_radius(radius), pos(m_pos), r(m_radius)
{
	m_primitiveType = PrimitiveType::NH_Sphere;
}

NonhierSphere::~NonhierSphere()
{
}

void NonhierSphere::hit(HitRecord &hr, Ray r, float t_0, float t_1) const
{
	hr.miss = false;

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
			t = roots[0];
			if (!(t > t_0 && t < t_1)) {
				// The solution is outside the interval so we don't accept it
				hr.miss = true;
			} else {
				hr.miss = false;
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
				hr.miss = false;
			} else if ( larger > t_0 && larger < t_1 ) {
				t = larger;
				hr.miss = false;
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

// Surface Group ---------------------------------------------------------------------------------
SurfaceGroup::SurfaceGroup(const std::list<Primitive *> & surfaces) : m_surfaces(surfaces)
{
	m_primitiveType = PrimitiveType::Group;
}

void SurfaceGroup::hit(HitRecord &hr, Ray r, float t_0, float t_1) const
{
	hr.miss = true;

	for (const Primitive *surface : m_surfaces)
	{
		HitRecord surface_hr;
		surface->hit(surface_hr, r, t_0, t_1);

		// If we hit the surface, update the record of the closest hit and
		// update the upper bound of our interval
		if (!surface_hr.miss) {
			hr = surface_hr;
			t_1 = hr.t;
		}
	}
}

std::ostream & operator << (std::ostream & os, const Primitive &p)
{
	char buffer[100]; // For string formatting
	switch(p.m_primitiveType)
	{
		case PrimitiveType::None:
			os << "Base primitive";
			break;
		case PrimitiveType::Sphere:
			os << "Sphere";
			break;
		case PrimitiveType::Cube:
			os << "Cube";
			break;
		case PrimitiveType::NH_Sphere:
		{
			const NonhierSphere *nh_sphere = static_cast<const NonhierSphere *>(&p);
			glm::vec3 position = nh_sphere->pos;
			double radius = nh_sphere->r;

			sprintf(buffer, "position:(%.2f,%.2f,%.2f) radius:%.2f", position.x, position.y, position.z, radius);
			os << "NH_Sphere:";
			os << buffer;
			break;
		}
		case PrimitiveType::NH_Box:
			os << "NH_Box";
			break;
		case PrimitiveType::Group:
			os << "Group";
			// Print each one in the group
			break;
		default:
			printf("%s | Error, default case reached\n", __func__);
			abort();
	}

	return os;

}
