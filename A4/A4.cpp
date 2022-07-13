#include <glm/ext.hpp>
#include <assert.h>
#include <iostream>
#include <stack>
#include "A4.hpp"
#include "GeometryNode.hpp"
#include "Ray.hpp"
#include "Primitive.hpp"
#include "PhongMaterial.hpp"
#define PLANE_WIDTH 100	
#define PLANE_HEIGHT 100
#define PLANE_DISTANCE 10
#define RAY_DISTANCE 2000

// Ray --------------------------------------------------------------------------
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

// Matrix Stack ------------------------------------------------------------------
// Convenience struct for handling the active transformation
// when walking the scene graph. Copied from A3
struct MatrixStack {
	MatrixStack();

	// Right multiplies the active transform by m. Requires
	// that this transformation is invertible (for calling pop())
	void push(const glm::mat4 &m); 

	// Removes the most recently pushed matrix
	void pop(); 

	// Empties the stack, resets active_transform to the identity
	void reset();

	// Matrix representing the product of all transformations
	// in the stack matrices
	glm::mat4 active_transform;
	std::stack<glm::mat4> matrices;
};

//----------------------------------------------------------------------------------------
MatrixStack::MatrixStack() : active_transform(glm::mat4(1.0f))
{

}

//----------------------------------------------------------------------------------------
void MatrixStack::push(const glm::mat4 &m)
{
	matrices.push(m);
	active_transform = active_transform*m;
}

//----------------------------------------------------------------------------------------
void MatrixStack::pop()
{
	glm::mat4 m_inverse = glm::inverse(matrices.top());

	// 'Undo' the matrix by right multiplying the active transform by the inverse
	active_transform = active_transform * m_inverse;

	// We don't need it anymore
	matrices.pop();
}

//----------------------------------------------------------------------------------------
void MatrixStack::reset()
{
	matrices = std::stack<glm::mat4>();
	active_transform = glm::mat4(1.0f);
}

// A few helper functions for walking the scene graph and preparing the primitives.

// We'll use mutual recursion like in A3
void processNodeList(std::list<SceneNode *> &nodes, std::list<Primitive *> &scene_surfaces, MatrixStack &ms);

void processNode(SceneNode &node, std::list<Primitive *> &scene_surfaces, MatrixStack &ms)
{
	switch(node.m_nodeType)
	{
		case NodeType::GeometryNode:
		{
			const GeometryNode *geometryNode = static_cast<const GeometryNode *>(&node);
			// Push the node's transform
			ms.push(geometryNode->get_transform());	

			// We also need to prepare the primitive with the right properties
			Primitive *surface = geometryNode->m_primitive;
			surface->m_material = geometryNode->m_material;
			surface->m_transform = ms.active_transform;

			scene_surfaces.push_back(surface);
			break;
		}
		case NodeType::SceneNode:
			// For the Scene node, we do nothing but push the local transform
			ms.push(node.get_transform());
			break;
		case NodeType::JointNode:
			// We aren't using joint nodes right now, so fall through to failure case
		default:
			printf("Default case reached, aborting");
			abort();
	}	

	// After rendering the node we process all its children
	// (make a copy first so we can safely modify the list in the mutual recursion)
	
	std::list<SceneNode *> children_copy = node.children;
	processNodeList(children_copy, scene_surfaces, ms);

	// We have processed all of its node's children, so we can safely pop off the transformation
	// local to this node
	ms.pop();
}

void processNodeList(std::list<SceneNode *> &nodes, std::list<Primitive *> &scene_surfaces, MatrixStack &ms)
{
	if(nodes.empty()) {
		return;
	} 

	SceneNode *first = nodes.front();
	nodes.pop_front();

	processNode(*first, scene_surfaces, ms);
	processNodeList(nodes, scene_surfaces, ms);
}

void printPercentDone(size_t current_col, size_t total_cols)
{

	static const int NUM_MARKERS(4);	
	static const float thresholds[NUM_MARKERS] = {0.25f,0.5f,0.75f,0.95f};
	static int print_threshold[NUM_MARKERS] = {0,0,0,0};
	static int threshold_called[NUM_MARKERS] = {0,0,0,0};

	float ratio = current_col / (float) total_cols;

	for(int i = 0; i < NUM_MARKERS; ++i) {
		if (ratio > thresholds[i]) {
			print_threshold[i] = 1;
		}
	}

	for(int i = 0; i < NUM_MARKERS; ++i) {
		if (print_threshold[i]) {
			if (threshold_called[i] == 0) {
				float percentage = 100*thresholds[i];
				printf("%.2f %% finished \n", percentage);
				threshold_called[i] = 1;
			}
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

	std::list<Primitive *> scene_surfaces;
	MatrixStack ms;

	// Prepare the primitives and their transformations by walking the scene graph
	processNode(*root, scene_surfaces, ms);

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

			printPercentDone(y, h);

			// Our image dimensions are w by h.
			// pixel position (i,j) in the image corresponds to the point (u,v) on the plane (where the ray passes through) and
			// we compute (u,v) as...

			float u = l + ((float) (r - l)*(x + 0.5f)) / w;
			float v = b + ((float) (t - b)*(y + 0.5f)) / h;
			v = -v; // negate v, otherwise the images come out flipped 

			// We construct our rays for a perspective view. The origin and direction are taken from textbook computations
			// (4.3 of Shirley's book)
		
			Ray r{eye, -PLANE_DISTANCE*w_cam + u*u_cam + v*v_cam};

			HitRecord hr;

			// Intersect the ray with all the surfaces
			surfaces.hit(hr, r, 0, RAY_DISTANCE);


			if (hr.miss) {
				// We missed. Just colour it black

				image(x, y, 0) = 0; //red
				image(x, y, 1) = 0; //green
				image(x, y, 2) = 0; //blue
			} else {
				// Compute the colour of the pixel, taking into account
				// the various point-light sources

				// Also take into account ambient lighting, leave this as a simple computation for now.
				// Ideally the factor multiplying the ambient intensity should be part of the material,
				// but that requires some changes to the skeleton assignment code that I would like to leave for later
				glm::vec3 colour = ambient * 0.5f;

				for ( const Light *light : lights )
				{
					// What happens if we exceed 1.0 ? ..
					colour = colour + light->illuminate(r, hr);
				}

				image(x, y, 0) = (double) colour.r;
				image(x, y, 1) = (double) colour.g;
				image(x, y, 2) = (double) colour.b;
			}

		}
	}

}
