#include <assert.h>
#include <iostream>
#include <stack>

#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>

#include "A4.hpp"
#include "A4_utils.hpp"
#include "GeometryNode.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"

#define EPSILON 0.01f
#define MAX_RECURSION_DEPTH 5
#define PLANE_HEIGHT 50
#define PLANE_WIDTH 50
#define PLANE_DISTANCE 50
#define SAMPLES_PER_PIXEL 10 
// #define ALIASING_A4

glm::vec3 shadeRay(const Ray &r, 
		float t0, 
		float t1, 
		const SurfaceGroup &scene_surfaces, 
		const glm::vec3 &ambient, 
		const std::list<Light *> &lights,
		int depth = 0)
{

	HitRecord hr;

	// Intersect the ray with all the surfaces
	scene_surfaces.hit(hr, r, t0, t1);

	// glm::vec3 background_colour = {0.0f, 0.8f, 0.95f}; sky blue
	
	// white
	glm::vec3 background_colour = {1.0, 1.0, 1.0}; 
	// glm::vec3 background_colour = {0.0f, 0.0f, 0.0f};
	// glm::vec3 background_colour = {0.7f, 0.7f, 0.7f};

	if (hr.miss) {

		return background_colour;

	} else {

		// Compute the colour of the pixel, taking into account
		// the various point-light sources

		// Also take into account ambient lighting, leave this as a simple computation for now.
		// Ideally the factor multiplying the ambient intensity should be part of the material,
		// but that seems to requires some changes to the skeleton assignment code that I would like to leave for later.

		static const float K_AMBIENT(0.8f);

		glm::vec3 colour = ambient*K_AMBIENT;

		for ( const Light *light : lights )
		{
			// Note: if we exceed 1.0f here (in any of the rgb channels) it is treated like 1.0f.
			colour = colour + light->illuminate(r, hr, scene_surfaces);
		}

		if (depth >= MAX_RECURSION_DEPTH) {

			return colour;

		} else {

			const glm::vec3 &o_mirror = hr.hit_point;

			const glm::vec3 &n = hr.n;

			glm::vec3 d_mirror = r.d - 2*glm::dot(r.d,n)*n;

			Ray r_mirror(o_mirror, d_mirror);

			PhongMaterial *phongMat = static_cast<PhongMaterial *>(hr.params->material);

			const glm::vec3 &k_mirror = phongMat->ks;

			glm::vec3 reflection_colour = shadeRay(r_mirror, EPSILON, RAY_DISTANCE_MAX, scene_surfaces, ambient, lights, depth + 1);

			if (glm::length2(reflection_colour - background_colour) < EPSILON) {
				// Discard background colour reflections
				// return colour;
			}

			return colour + glm::vec3{k_mirror.r * reflection_colour.r, k_mirror.g * reflection_colour.g, k_mirror.b * reflection_colour.b};
		}

	}

}

// The actual ray tracing function
void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights)
{
	// Construct the orthonormal coordinate / camera frame {u,v,w} from eye,view,up
	
	// We choose w to be opposite the view vector, and all rays to have direction -w
	glm::vec3 w_cam{-1 * glm::normalize(view)};
	glm::vec3 u_cam{glm::normalize(glm::cross(up,w_cam))};
	// This chooses 'v' so that it is the closest one to 'up' that is orthogonal to w
	glm::vec3 v_cam{glm::cross(w_cam,u_cam)};

	std::cout << "Calling A4_Render(\n" <<
		"\t" << *root <<
		"\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
		"\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		"\t" << "view: " << glm::to_string(view) << std::endl <<
		"\t" << "up:   " << glm::to_string(up) << std::endl <<
		"\t" << "fovy: " << fovy << std::endl <<
		"\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		"\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	// Walk the scene graph and collect the surfaces with their corresponding transformations
	std::list<Primitive *> scene_primitives;
	std::list<SurfaceParams> surface_parameters;
	MatrixStack ms{};

	processNode(*root, scene_primitives, surface_parameters, ms);

	SurfaceGroup surfaces(scene_primitives, surface_parameters);


#ifndef NDEBUG
	for (const Primitive *p : surfaces.m_surfaces) 
	{
		std::cout << *p << std::endl;	
		//std::cout << p->m_transform << std::endl;
	}
#endif

	// Now we need to define the dimensions of our image plane, for 
	// computing ray directions

	static const float l(-PLANE_WIDTH/2);
	static const float r(PLANE_WIDTH/2);
	static const float t(PLANE_HEIGHT/2);
	static const float b(-PLANE_HEIGHT/2);

// Iterate through the pixels this way, avoiding the double for-loop and
// making it more natural to parallelize

#pragma omp parallel for schedule(dynamic, 3)
	for (int k = 0; k < w*h; ++k) 
	{
		uint x = k % w;
		uint y = k / h;

#ifndef ALIASING_A4 
		// Our image dimensions are w by h.
		// pixel position (i,j) in the image corresponds to the point (u,v) on the plane (where the ray passes through) and
		// we compute (u,v) as...

		float u = l + ((float) (r - l)*(x + 0.5f)) / w;
		float v = b + ((float) (t - b)*(y + 0.5f)) / h;
		v = -v; // negate v, otherwise the images come out flipped 

		// We construct our rays for a perspective view. The origin and direction are taken from textbook computations
		// (4.3 of Shirley's book)		
		Ray r{eye, -PLANE_DISTANCE*w_cam + u*u_cam + v*v_cam};

		glm::vec3 colour = shadeRay(r, 0, RAY_DISTANCE_MAX, surfaces, ambient, lights, 0);

		image(x, y, 0) = colour.r;
		image(x, y, 1) = colour.g;
		image(x, y, 2) = colour.b;
#else
		float u,v;
		glm::vec3 colour; 

		for (int s = 0; s < SAMPLES_PER_PIXEL; ++s)
		{
			u = l + ((float) (r - l)*(x + random_double())) / w;
			v = b + ((float) (t - b)*(y + random_double())) / h;
			v = -v; // negate v, otherwise the images come out flipped 

			// We construct our rays for a perspective view. The origin and direction are taken from textbook computations
			// (4.3 of Shirley's book)		
			Ray r{eye, -PLANE_DISTANCE*w_cam + u*u_cam + v*v_cam};

			colour = colour + shadeRay(r, 0, RAY_DISTANCE_MAX, surfaces, ambient, lights, 0);
		}

		float scale = 1.0 / SAMPLES_PER_PIXEL;
		colour = colour*scale;

		image(x, y, 0) = colour.r;
		image(x, y, 1) = colour.g;
		image(x, y, 2) = colour.b;

#endif // ALIASING_A4
	}
}
