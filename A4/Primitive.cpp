#include "Primitive.hpp"
#include "polyroots.hpp"
#include <glm/ext.hpp>
#include <limits>

Primitive::Primitive() : m_primitiveType(PrimitiveType::None)
{

}

Primitive::~Primitive()
{
}

void Primitive::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const
{
	// Default behaviour is just to return a miss
	hr.p = this;
	hr.miss = true;
	return;
}

void transformVector(glm::vec3 &v, const glm::mat4 &m) 
{
	glm::vec4 u = m * glm::vec4{v, 1.0f};

	if (u.w == 0) {
		printf("%s | Error, cannot homogenize vector \n", __func__);
		// Can't homogenize the vector so just bail out.
		return;
	} 

	v = glm::vec3{u.x/u.w, u.y/u.w, u.z/u.w};
}

void Primitive::hit(HitRecord &hr, const Ray &r, float t_0, float t_1, const SurfaceParams &sp) const
{
	const glm::mat4 &m = sp.trans;
	const glm::mat4 &inv = sp.inv_trans;

	// First we construct the ray with the inverse transformation left-multiplied 
	Ray r_t(r.o, r.d);
	r_t.transform(inv);

	// Now hit the base object with the transformed ray	
	hitBase(hr, r_t, t_0, t_1);

	if (!hr.miss) {
		// Get the matrix for transforming normals
		glm::mat3 normalTransform = glm::mat3(glm::transpose(inv));
		hr.n = glm::normalize(normalTransform * hr.n);

		// Transform the intersection point also 
		transformVector(hr.hit_point, m);

		hr.params = &sp;

		updateTextureCoordinates(hr);

		// The nice thing is we do not need to change hr.t .
	} else {
		// we did not hit the base object with the transformed ray.
		// so we say that the ray did not hit the transformed object.
		hr.miss = true;
	}
}

void Primitive::updateTextureCoordinates(HitRecord &hr) const
{
	hr.u = 0.0;
	hr.v = 0.0;
}

// Sphere ----------------------------------------------------------------------------

Sphere::Sphere() : m_nh_sphere(NonhierSphere({0,0,0}, 1.0))
{
}

void Sphere::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const 
{
	m_nh_sphere.hitBase(hr, r, t_0, t_1);
}

Sphere::~Sphere()
{

}

// Cube ----------------------------------------------------------------------------
Cube::Cube() : m_nh_box(NonhierBox({0,0,0},{1,1,1}))
{

}

Cube::~Cube()
{

}

void Cube::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const 
{
	m_nh_box.hitBase(hr, r, t_0, t_1);
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

void NonhierSphere::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const
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
		hr.hit_point = intersection_point;
	}
}

// Nonhier Box --------------------------------------------------------------------
NonhierBox::NonhierBox(const glm::vec3& pos, double size)
	: m_min(pos), m_max(pos + glm::vec3(size, size, size))
{
	m_primitiveType = PrimitiveType::NH_Box;
}

NonhierBox::NonhierBox(const glm::vec3& bmin, const glm::vec3 &bmax)
	: m_min(bmin), m_max(bmax)
{
	m_primitiveType = PrimitiveType::NH_Box;
}

bool approx(float a, float b)
{
	static const float EPSILON_APPROX(0.3f);
	return abs(a - b) < EPSILON_APPROX;
}

// TODO : I think this function is broken, need to fix, but not essential right now since
// images seem to be rendering still
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

	if (approx(p.z, m_min.z)) {
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

	return {0,0,0};

}

void NonhierBox::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const 
{

	float tmin = t_0;
	float tmax = t_1;

	bool hit = true;

	for (int i = 0; i < 3; ++i) {

		if (r.d[i] != 0.0) {
		    float t1 = (m_min[i] - r.o[i])/r.d[i];
		    float t2 = (m_max[i] - r.o[i])/r.d[i];

		    tmin = std::max(tmin, std::min(t1, t2));
		    tmax = std::min(tmax, std::max(t1, t2));
		} else if (r.o[i] <= m_min[i] || r.o[i] >= m_max[i]) {
			hit = false;
		}
	}

	hit = hit && (tmax > tmin && tmax > 0.0f);

	if (hit) {

		hr.p = this;
		hr.miss = false;

		hr.t = tmin;
		hr.hit_point = r.evaluate(tmin);
		hr.n = computeNormal(hr.hit_point);

		// For now, let's return even if we get a 0 normal and we hit inside the box.
		// I'm doing this because of bounding boxes.
		return;

	}

	// If we are here, we missed.	

	hr.p = this;
	hr.t = -1;
	hr.miss = true;

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
SurfaceGroup::SurfaceGroup(const std::list<Primitive *> & surfaces, const std::list<SurfaceParams> &params) : 
	m_surfaces(surfaces), m_surfaceParams(params)

{
	m_primitiveType = PrimitiveType::Group;
}

void SurfaceGroup::hitBase(HitRecord &hr, const Ray &r, float t_0, float t_1) const
{
	assert(m_surfaces.size() == m_surfaceParams.size());

	hr.miss = true;

	std::list<Primitive *>::const_iterator i = m_surfaces.begin();
	std::list<SurfaceParams>::const_iterator j = m_surfaceParams.begin();

	while (i != m_surfaces.end())
	{
		const Primitive *surface = *i;
		const SurfaceParams &sp = *j;

		HitRecord surface_hr;
		surface->hit(surface_hr, r, t_0, t_1, *j);

		// If we hit the surface, update the record of the closest hit and
		// update the upper bound of our interval
		if (!surface_hr.miss) {
			hr = surface_hr;
			t_1 = hr.t;
		}

		++i;
		++j;
	}

}

void SurfaceGroup::hit(HitRecord &hr, const Ray &r, float t_0, float t_1, const SurfaceParams &sp) const
{
	hitBase(hr, r, t_0, t_1);
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
