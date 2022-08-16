#ifndef A4_UTILS_H
#define A4_UTILS_H
#include <random>
#include <stack>
#include <glm/glm.hpp>

// Matrix Stack ------------------------------------------------------------------
// Convenience struct for handling the active transformation
// when walking the scene graph. Copied from A3 
struct MatrixStack
{
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

			SurfaceParams sp;
			sp.material = static_cast<PhongMaterial *>(geometryNode->m_material);
			sp.name = geometryNode->m_name;
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

// For antialiasing
double random_double() 
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);	
}

#endif //A4_UTILS_H
