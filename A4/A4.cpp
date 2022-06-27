#include <glm/ext.hpp>
#include <assert.h>
#include <iostream>
#include "A4.hpp"
#include "GeometryNode.hpp"
#include "Ray.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#define PLANE_WIDTH 100	
#define PLANE_HEIGHT 100
#define PLANE_DISTANCE 10

// Implement the ray class here

Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction) 
	: m_origin(origin), m_direction(direction), o(m_origin), d(m_direction)
{
	static const float EPSILON_RAY = 0.1f;
	// Want to make sure our direction is non-zero. I don't want to 
	// worry about rays with a null direction, because that's just a point
	assert(glm::length2(direction) > EPSILON_RAY^2);
}

glm::vec3 Ray::evaluate(float t) const
{
	return o + t*d;
}

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

	//TODO : It is a good idea to clean this code up with helper functions, but I will do it after
	// I am more certain about the structure of this function

	// Get the list of surfaces we need to hit.
	
	// For now we'll assume that the root node has its children a list of
	// geometry node corresponding to non-hierarchical surfaces 

	std::list<Primitive *> scene_surfaces;

	for (const SceneNode *node : root->children) {

		if (node->m_nodeType != NodeType::GeometryNode) {
			continue;
		}

		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);	

		Primitive *surface = geometryNode->m_primitive;
		surface->m_material = geometryNode->m_material;

		scene_surfaces.push_back(surface);
	}

	SurfaceGroup surfaces(scene_surfaces);

#ifndef NDEBUG
	for (const Primitive *p : surfaces.m_surfaces) 
	{
		std::cout << *p << std::endl;
	}
#endif

	// Now we need to define the dimensions of our image plane, for 
	// computing ray directions

	static const float l(-PLANE_WIDTH/2);
	static const float r(PLANE_WIDTH/2);
	static const float t(PLANE_HEIGHT/2);
	static const float b(-PLANE_HEIGHT/2);

#ifndef NDEBUG
	printf("Ray origin:(%f,%f,%f)\n", eye.x, eye.y, eye.z);
#endif

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {

			// Our image dimensions are w by h.
			// pixel position (i,j) in the image corresponds to the point (u,v) on the plane (where the ray passes through) and
			// we compute (u,v) as...

			float u = l + ((float) (r - l)*(x + 0.5f)) / w;
			float v = b + ((float) (t - b)*(y + 0.5f)) / h;

			// We construct our rays for a perspective view. The origin and direction are taken from textbook computations
			// (4.3 of Shirley's book)
		
			Ray r(eye, -PLANE_DISTANCE*w_cam + u*u_cam + v*v_cam);

			// Intersect the ray with all the surfaces
			HitRecord hr;
			// What is a good interval for our ray?...
			surfaces.hit(hr, r, 0, 2000);


			if (x == 127 && y == 127) {
				printf("HERE\n");
			}
			if (hr.miss) {
				// We missed. Just colour it black

				image(x, y, 0) = 0; //red
				image(x, y, 1) = 0; //green
				image(x, y, 2) = 0; //blue
			} else {
				// Compute the colour of the pixel, taking into account
				// the various point-light sources

				glm::vec3 colour;

				for ( const Light *light : lights )
				{
					colour = colour + light->illuminate(r, hr);
				}

				image(x, y, 0) = (double) colour.r;
				image(x, y, 1) = (double) colour.g;
				image(x, y, 2) = (double) colour.b;
			}

		}
	}

}
