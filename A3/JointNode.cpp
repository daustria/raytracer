#include "JointNode.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
	
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
}

//--------------------------------------------------------------------------------------
void JointNode::rotate(char axis, float angle)
{
	// The implementation of this rotation is the same as the parent class except we 
	// clamp values on x and y axis
	glm::vec3 rot_axis;

	switch(axis)
	{
		case 'x':
			// clamp the angle before rotating
			if (angle > m_joint_x.max) {
				angle = m_joint_x.max;
			} else if (angle < m_joint_x.min) {
				angle = m_joint_x.min;
			} 
			rot_axis = glm::vec3(1.0f, 0, 0);

			break;
		case 'y':

			// clamp the angle before rotating
			if (angle > m_joint_y.max) {
				angle = m_joint_y.max;
			} else if (angle < m_joint_x.min) {
				angle = m_joint_y.min;
			} 

			rot_axis = glm::vec3(0, 1.0f, 0);

			break;
		case 'z':
			rot_axis = glm::vec3(0, 0, 1.0f);
			break;
	}

	glm::mat4 rotate = glm::rotate(glm::radians(angle), rot_axis);

	trans = rotate*trans;
}
