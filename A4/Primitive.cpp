#include "Primitive.hpp"
#include "polyroots.hpp"
#include <limits>

static const float EPSILON = 0.01f;

Primitive::Primitive() : m_primitiveType(PrimitiveType::None), m_material(nullptr)
{

}

Primitive::~Primitive()
{
}

void Primitive::hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const
{
	// Default behaviour is just to return a miss
	hr.p = this;
	hr.miss = true;
	return;
}

void transformVertex(glm::vec3 &v, const glm::mat4 &m) 
{
	glm::vec4 v_hom = {v.x, v.y, v.z, 1.0f};
	v_hom = m*v_hom;

	if (v_hom.w == 0) {
		// Can't homogenize the vector so just bail out.
		return;
	} 

	v = {v_hom.x/v_hom.w, v_hom.y/v_hom.w, v_hom.z/v_hom.w};
}

void Primitive::transformPrimitive(const glm::mat4 &m)
{
	// Default behaviour is to do nothing. The inheriting primitives
	// should override this function to apply the matrix m to their vertices
}

// Sphere ----------------------------------------------------------------------------
Sphere::~Sphere()
{

}

// Cube ----------------------------------------------------------------------------
Cube::~Cube()
{

}

// Nonhier Sphere --------------------------------------------------------------------
NonhierSphere::NonhierSphere(const glm::vec3& position, double radius)
	: m_pos(position), m_radius(radius), pos(m_pos), r(m_radius)
{
	m_primitiveType = PrimitiveType::NH_Sphere;
}

NonhierSphere::~NonhierSphere()
{
}

void NonhierSphere::transformPrimitive(const glm::mat4 &m)
{
	transformVertex(m_pos, m);
}

void NonhierSphere::hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const
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
		hr.n = glm::normalize(sphere_normal);
	}
}


// Nonhier Box --------------------------------------------------------------------
NonhierBox::NonhierBox(const glm::vec3& pos, double size)
	: m_pos(pos), m_size(size), m_min(m_pos), m_max(m_pos + glm::vec3(m_size))
{
	m_primitiveType = PrimitiveType::NH_Box;
}

bool approx(float a, float b)
{
	return abs(a - b) < EPSILON;
}

glm::vec3 NonhierBox::computeNormal(const glm::vec3 &p) const
{
	// The cube has 6 planes, each with a different normal (need it to point outside the surface)
	// The planes are:
	//
	// x = min.x		x = max.x
	// y = min.y		y = max.y
	// z = min.z		z = max.z

	if (approx(p.x, m_min.x)) {
		return {-1.0f, 0, 0};
	}

	if (approx(p.y, m_min.y)) {
		return {0, -1.0f, 0};
	}

	if (approx(p.y, m_min.z)) {
		return {0, 0, 1.0f};
	}

	if (approx(p.x, m_max.x)) {
		return {1.0f, 0, 0};
	}

	if (approx(p.y, m_max.y)) {
		return {0, 1.0f, 0};
	}

	if (approx(p.z, m_max.z)) {
		return {0, 0, -1.0f};
	}

	printf("%s | Error: no normal computed for point {%.2f,%.2f,%.2f}\n", __func__, p.x, p.y, p.z);
	return {0,0,0};

}

void NonhierBox::hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const 
{

	// Implementation of box primitives seems difficult and not much
	// official resources. I think I'll just leave box as triangle meshes
	// and worry about ray triangle intersections
	float tmin = 0;
	float tmax = std::numeric_limits<float>::infinity();

	glm::vec3 bmin = m_min;
	glm::vec3 bmax = m_max;

	for (int i = 0; i < 3; ++i) {
		// Note : we assume our rays always have non-zero direction before
		// trying to intersect them

		float t1 = (bmin[i] - r.o[i])/r.d[i];
		float t2 = (bmax[i] - r.o[i])/r.d[i];

		tmin = std::max(tmin, std::min(t1, t2));
		tmax = std::min(tmax, std::max(t1, t2));
	}

	bool hit = tmax > tmin;

	if (hit) {
		hr.p = this;
		hr.t = tmin;
		hr.miss = false;
		hr.n = computeNormal(r.evaluate(tmin));
	} else {
		hr.p = this;
		hr.t = -1;
		hr.miss = true;
	}

}

void NonhierBox::transformPrimitive(const glm::mat4 &m) 
{
	transformVertex(m_pos, m);
	m_min = m_pos;
	m_max = m_pos + glm::vec3(m_size);
}

const glm::vec3 &NonhierBox::bmin() const
{
	return m_min;
}

const glm::vec3 &NonhierBox::bmax() const
{
	return m_max;
}

NonhierBox::~NonhierBox()
{

}

// Surface Group ---------------------------------------------------------------------------------
SurfaceGroup::SurfaceGroup(const std::list<Primitive *> & surfaces) : m_surfaces(surfaces)
{
	m_primitiveType = PrimitiveType::Group;
}

void SurfaceGroup::hit(HitRecord &hr, const Ray &r, float t_0, float t_1) const
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
		{
			const NonhierBox *nh_box = static_cast<const NonhierBox *>(&p);
			glm::vec3 bmin{nh_box->bmin()};
			glm::vec3 bmax{nh_box->bmax()};

			sprintf(buffer, "min:(%.2f,%.2f,%.2f) max:(%.2f,%.2f,%.2f)", bmin.x, bmin.y, bmin.z, bmax.x, bmax.y, bmax.z);
			os << "NH_Box:";
			os  << buffer;
			break;
		}
		case PrimitiveType::Group:
			os << "Group";
			// Print each one in the group
			break;
		case PrimitiveType::Mesh:
			os << "Mesh";
			break;
		default:
			printf("%s | Error, default case reached\n", __func__);
	}

	return os;

}
