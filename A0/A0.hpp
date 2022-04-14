#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

class A0 : public CS488Window {
public:
	A0();
	virtual ~A0();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void uploadTriangleDataToVbo();
	void mapVboDataToShaderAttributeLocation();
	void uploadUniformsToShader();

	void resetShape();

	GLuint m_vao_triangle; // Vertex Array Object
	GLuint m_vbo_triangle; // Vertex Buffer Object
	ShaderProgram m_shader;

	// Red,Blue,Green color components of shape.
	// Individual elements can be accessed in various ways:
	// vec3[0], vec3[1], vec3[2]
	// vec3.x, vec3.y, vec3.z
	// vec3.r, vec3.g, vec3.b
	glm::vec3 m_shape_color;

	// Shape rotation angle in radians.
	float m_shape_rotation;

	// 2-component vector of floats.
	// Shape translation in (x,y) direction.
	glm::vec2 m_shape_translation;

	// Positive float range for size of shape.
	// Clamped to be within [0.0, 10.0].
	// Shape starts at size 1.0
	float m_shape_size;

	// 2-component vector of floats.
	// Represents mouse position in GL coordinates.
	// Coordinates span [-1,1] along x and y directions within window.
	glm::vec2 m_mouse_GL_coordinate;

	// True if mouse button is currently down.
	bool m_mouseButtonActive;
};
