#pragma once

#include "SceneNode.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode(
		const std::string & meshId,
		const std::string & name
	);

	Material material;

	// Mesh Identifier. This must correspond to an object name of
	// a loaded .obj file.
	const std::string meshId;

	// same as scene node scale, but we dont push it onto the stack, only keep the scale local.
	virtual void scale(const glm::vec3& amount) override;
	virtual void rotate(char axis, float angle) override;
	// same as scene node translate, but we dont push it onto the stack, only keep the scale local.
	virtual void translate(const glm::vec3& amount) override;

	const glm::mat4 &get_transform_local() const;
	glm::mat4 trans_local; // the transform to apply only at this node. does not pass over to its children
};
