#include <glm/ext.hpp>
#include <assert.h>
#include "A4.hpp"
#include "GeometryNode.hpp"
#include "Ray.hpp"
#include "Primitive.hpp"
#define PLANE_WIDTH 100	
#define PLANE_HEIGHT 100
#define PLANE_DISTANCE 10

// Implement the ray class here

Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction) 
	: o(origin), d(direction), origin(origin), direction(direction)
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

	// For now we'll assume that the root node has its children a list of
	// geometry node corresponding to non-hierarchical surfaces we can hit 

	// Let's also assume that the surface we're going to hit is just going to be a sphere, and it corresponds to
	// the only child of the root node, which we assume to be a geometry node
	SceneNode * node = root->children.front();
	const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);
	const Primitive *p = geometryNode->m_primitive;

	const NonhierSphere *nh_sphere = static_cast<const NonhierSphere *>(p);

	// Now we need to define the dimensions of our image plane, for 
	// computing ray directions

	static const float l(-PLANE_WIDTH/2);
	static const float r(PLANE_WIDTH/2);
	static const float t(PLANE_HEIGHT/2);
	static const float b(-PLANE_HEIGHT/2);

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {

			// Our image dimensions are w by h.
			// pixel position (i,j) in the image corresponds to the point (u,v) on the pixel plane, and
			// we compute (u,v) as...

			float u = l + ((float) (r - l)*(x + 0.5f)) / w;
			float v = b + ((float) (t - b)*(x + 0.5f)) / h;

			// We construct our rays for a perspective view. The origin and direction are taken from textbook computations
			// (4.3 of Shirley's book)

			Ray r(eye, -PLANE_DISTANCE*w_cam + u*u_cam + v*v_cam);


			// Red: 
			image(x, y, 0) = (double)1.0;
			// Green: 
			image(x, y, 1) = (double)1.0;
			// Blue: 
			image(x, y, 2) = (double)1.0;
		}
	}

}
