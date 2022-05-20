#include "GeometryNode.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
		const std::string & meshId,
		const std::string & name
)
	: SceneNode(name),
	  meshId(meshId)
{
	m_nodeType = NodeType::GeometryNode;
}

//---------------------------------------------------------------------------------------
void GeometryNode::scale(const glm::vec3& amount)
{
	trans_local = glm::scale(amount) * trans_local;
	// trans stays the same
}
//---------------------------------------------------------------------------------------
void GeometryNode::rotate(char axis, float angle) 
{
	glm::vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = glm::vec3(1,0,0);
			break;
		case 'y':
			rot_axis = glm::vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = glm::vec3(0,0,1);
	        break;
		default:
			break;
	}
	glm::mat4 rot_matrix = glm::rotate(glm::radians(angle), rot_axis);
	trans = rot_matrix * trans;
	trans_local = rot_matrix * trans_local;
}

//---------------------------------------------------------------------------------------
void GeometryNode::translate(const glm::vec3& amount) 
{
	glm::mat4 translation = glm::translate(amount);
	trans = translation * trans;
	trans_local = translation * trans_local;
}

//---------------------------------------------------------------------------------------
const glm::mat4 &GeometryNode::get_transform_local() const
{
	return trans_local;

}
