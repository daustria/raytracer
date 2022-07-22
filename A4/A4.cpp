#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>
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
#define PLANE_DISTANCE 100

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

// Matrix Stack ------------------------------------------------------------------
// Convenience struct for handling the active transformation
// when walking the scene graph. Copied from A3 
struct MatrixStack {
	// TODO : Make this class keep inverse transformations. Or 
	// come up with another solution so that we don't compute inverse transformation matrices
	// when we do the ray object intersection 
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
	glm::mat4 active_inverse;
	std::stack<glm::mat4> matrices;
	std::stack<glm::mat4> inverses;
};

//----------------------------------------------------------------------------------------
MatrixStack::MatrixStack() : active_transform(glm::mat4(1.0f)), active_inverse(glm::mat4(1.0f))
{

}

//----------------------------------------------------------------------------------------
void MatrixStack::push(const glm::mat4 &m)
{
	glm::mat4 inv = glm::mat4(m);

	matrices.push(m);
	inverses.push(inv);
	active_transform = active_transform*m;
	active_inverse = inv * active_inverse;
}

//----------------------------------------------------------------------------------------
void MatrixStack::pop()
{
	glm::mat4 m_inverse = glm::inverse(matrices.top());

	// 'Undo' the matrix by right multiplying the active transform by the inverse
	active_transform = active_transform * m_inverse;

	active_inverse = matrices.top() * active_inverse;

	// We don't need it anymore
	matrices.pop();
}

//----------------------------------------------------------------------------------------
void MatrixStack::reset()
{
	matrices = std::stack<glm::mat4>();
	inverses = std::stack<glm::mat4>();
	active_transform = glm::mat4(1.0f);
	active_inverse = glm::mat4(1.0f);
}

// A few helper functions for walking the scene graph and preparing the primitives.

void processNodeList(std::list<SceneNode *> &nodes, std::list<Primitive *> &scene_primitives, std::list<SurfaceParams> &surface_parameters, MatrixStack &ms);

void processNode(SceneNode &node, std::list<Primitive *> &scene_primitives, std::list<SurfaceParams> &surface_parameters, MatrixStack &ms)
{
	// First thing is to push the node's local transform 
	ms.push(node.get_transform());	

	switch(node.m_nodeType)
	{
		case NodeType::GeometryNode:
		{
			const GeometryNode *geometryNode = static_cast<const GeometryNode *>(&node);

			// We also need to prepare the primitive with the right properties


			// Error: these Primitive pointers are shared. I need to change this so that either I walk the scene graph on each render,
			// or I make a new Primitive.
			Primitive *surface = geometryNode->m_primitive;

			// This code here should probably altered slightly since now we know that primitive pointers are shared, and
			// our work here could be overwritten
			surface->m_material = geometryNode->m_material;
			surface->m_name = geometryNode->m_name;

			// Store the transformations right into the primitives.
			// I'm not sure if this is the best way to do this, we could alternatively walk
			// the scene graph for every ray, avoiding having to store transformations.

			SurfaceParams sp;
			sp.trans = ms.active_transform;
			sp.inv_trans = glm::inverse(ms.active_transform);

			surface_parameters.push_back(sp);	
			scene_primitives.push_back(surface);

			break;
		}
		case NodeType::SceneNode:
			// Do nothing
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
	processNodeList(children_copy, scene_primitives, surface_parameters, ms);

	// We have processed all of its node's children, so we can safely pop off the transformation
	// local to this node
	ms.pop();
}

void processNodeList(std::list<SceneNode *> &nodes, std::list<Primitive *> &scene_primitives, std::list<SurfaceParams> &surface_parameters, MatrixStack &ms)
{
	if(nodes.empty()) {
		return;
	} 

	SceneNode *first = nodes.front();
	nodes.pop_front();

	processNode(*first, scene_primitives, surface_parameters, ms);
	processNodeList(nodes, scene_primitives, surface_parameters, ms);
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

#ifndef NDEBUG
	// printf("Ray origin:(%f,%f,%f)\n", eye.x, eye.y, eye.z);
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
			surfaces.hit(hr, r, 0, RAY_DISTANCE_MAX);
			
			// some debugging code.. i just change x and y to be the pixels i want
			if ( x == 250 && y == 250 ) {
				//printf("HERE\n");
			}

			// Primitive *front = surfaces.m_surfaces.front();
			// front->hit(hr, r, 0, RAY_DISTANCE);

			if (hr.miss) {
				// We missed. Just colour it black

				image(x, y, 0) = 0.3f;
				image(x, y, 1) = 0.3f;
				image(x, y, 2) = 1.0f;
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
					// Note: if we exceed 1.0f here, it treats it like 1.0f
					colour = colour + light->illuminate(r, hr, surfaces);
				}

				image(x, y, 0) = (double) colour.r;
				image(x, y, 1) = (double) colour.g;
				image(x, y, 2) = (double) colour.b;
			}

		}
	}

}
