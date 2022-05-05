#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <stdlib.h>
#include <map>

#include "ViewAdj.hpp"

#define SCREEN_WIDTH 768
#define SCREEN_HEIGHT 768

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;

// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
struct VertexData {

	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};

//Map storing whether keys are held or not

class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

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
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	// Takes the line pq in model coordinates, applies the matrix transformations and clipping
	// to get the two vertices of the corresponding line in normalized device coordinates (NDC) [-1,1]^2
	std::pair<glm::vec2, glm::vec2> processLine(const glm::vec4 &p, const glm::vec4 &q, bool omit_model_transform, bool omit_model_scale);

	void initCubeVertices();
	void initCubeIndices();

	void initCoordinateFrameVertices();

	void initMatrices();
	void updateModelMatrix();

	void initCoordinateSystems();


	glm::vec4 transformVertexProjection(glm::vec3 vertex, bool print_data=false);

	// Updates the world matrix, with the new rotation, translation, and scale factor set by the user 
	void updateWorldMatrix();

	// orthographic projection
	// left and right planes will be x=r, x=-r
	// top and bottom planes will be y=t, y=-t
	// near and far planes will be z=n, z=f where we imagine n,f < 0
	void updateProjectionMatrix(float r, float t, float n, float f);

	void updateCameraMatrix();
	
	void reset(); // initialize the relevant variables to their starting values

	void adjustCoordinateAxes(float horizontal_mouse_offset, bool offset_sign);

	ShaderProgram m_shader;/* Actual program that we attach the shaders to, and link them */

	// Keep the individual rotations for transforming the coordinate frames
	glm::mat4 m_model_translate;
	glm::mat4 m_model_rotate;
	glm::mat4 m_model_scale;

	glm::mat4 m_model; // object space to world space
	glm::mat4 m_model_no_scale;

	glm::mat4 m_camera; // world space to camera space

	glm::mat4 m_orth_proj; //orthogonal projection matrix
	glm::mat4 m_persp; //perspective matrix

	glm::mat4 m_proj; // projection, product of orth_proj and perspective matrix

	glm::mat4 m_viewport; //canonical-view-volume to screenspace 
	glm::mat4 m_final; //stores the product of the above matrices

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	std::vector<glm::vec3> m_cubeVertices; //this is the data we'll transform to screen coordinates. starts in model coordinates
	std::vector<int> m_cubeLineIndices; // holds indices of vertices, describing the order we draw the lines of the cube

	// For the coordinat frames, we'll just store lines since each coordinate frame only has 3 lines.
	// No need to hold indices of vertices to describe only 3 lines.
	std::vector<std::pair<glm::vec3, glm::vec3>> m_coordinateFrameLines;

	// For implementing the transformations along various coordinate systems..
	// Model Space
	ViewAdjustor m_scaleObjAdj; //adjust size of object in world space
	ViewAdjustor m_translateObjAdj; //adjust origin of object in world space
	ViewAdjustor m_rotateObjAdj; //adjust rotation of object in world space

	// Eye Space
	ViewAdjustor m_translateEyeAdj;
	ViewAdjustor m_rotateEyeAdj;


	ViewAdjustor m_perspectiveAdj;

	// Stores whether a GLFW key is being held
	std::map<int, bool> m_keyMap;

	//Tracking horizontal movement of the mouse
	double m_oldMouseX = 0.0f;

	bool m_printLineDebugInfo = true;
};

// divides all coordinates by the 4th, assumes it is non-zero
void homogenize4thChart(glm::vec4 &);
