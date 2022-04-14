#include "A0.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <math.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

using namespace glm;
using namespace std;

const float PI = 3.14159265f;


//----------------------------------------------------------------------------------------
// Constructor
A0::A0()
	: m_shape_color(glm::vec3(1.0f, 1.0f, 1.0f)),
	  m_shape_translation(vec2(0.0f)),
	  m_shape_size(1.0f),
	  m_shape_rotation(0.0f),
	  m_mouse_GL_coordinate(dvec2(0.0)),
	  m_mouseButtonActive(false)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A0::~A0()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A0::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	enableVertexAttribIndices();

	uploadTriangleDataToVbo();

	mapVboDataToShaderAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A0::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A0::enableVertexAttribIndices()
{
	glGenVertexArrays(1, &m_vao_triangle);
	glBindVertexArray(m_vao_triangle);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A0::uploadTriangleDataToVbo()
{
	// Vertex positions for triangle vertices.
	// Default OpenGL coordinates range from -1 to 1 in all directions (x,y,z).
	float half_sqrt3 = float(std::sqrt(3.0)) * 0.5f;
	vec3 triangleVertices[] = {
		// Construct equalaterial triangle
		vec3(0.25f, 0.0f, 0.0f),
		vec3(0.25f, half_sqrt3 * 0.5f, 0.0),
		vec3(0.5f, 0.0f, 0.0f)
	};

	// Construct triangle centroid and move vertices so triangle is centered at origin
	vec3 centroid(0.0f);
	for(const vec3 & v : triangleVertices) {
		centroid += v;
	}
	centroid /= 3.0f;
	for(vec3 & v : triangleVertices) {
		v -= centroid;
	}


	// Generate a vertex buffer object to hold the triangle's vertex data.
	glGenBuffers(1, &m_vbo_triangle);

	//-- Upload triangle vertex data to the vertex buffer:
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices,
			GL_STATIC_DRAW);


	// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A0::mapVboDataToShaderAttributeLocation()
{
	glBindVertexArray(m_vao_triangle);

	// Tell GL how to map data from the vertex buffer "m_vbo_triangle" into the
	// "position" vertex attribute index of our shader program.
	// This mapping information is stored in the Vertex Array Object "m_vao_triangle".
	// That is why we must bind "m_vao_triangle" first in the line above, so
	// that "m_vao_triangle" captures the data mapping issued by the call to
	// glVertexAttribPointer(...).
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_triangle);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A0::uploadUniformsToShader()
{
	m_shader.enable();
		// Query the shader program for the location number of the uniform named "color".
		GLint uniformLocation_colour = m_shader.getUniformLocation("colour");

		// Set the uniform's value.
		glUniform3f(uniformLocation_colour, m_shape_color.r, m_shape_color.g,
				m_shape_color.b);

		vec3 z_axis(0.0f,0.0f,1.0f);
		mat4 transform = glm::translate(mat4(), vec3(m_shape_translation, 0.0f));
		transform *= glm::scale(mat4(), vec3(m_shape_size));
		transform *= glm::rotate(mat4(), m_shape_rotation, z_axis);

		GLint uniformLocation_modelMatrix = m_shader.getUniformLocation("transform");
		glUniformMatrix4fv(uniformLocation_modelMatrix, 1, GL_FALSE,
				glm::value_ptr(transform));

	m_shader.disable();

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A0::appLogic()
{
	// Place per frame, application logic here ...


	// Clamp shape size to be within a reasonable range
	m_shape_size = glm::clamp(m_shape_size, 0.0f, 10.0f);

	uploadUniformsToShader();
}


void A0::resetShape()
{
	m_shape_color.r = 1.0f;
	m_shape_color.g = 1.0f;
	m_shape_color.b = 1.0f;
	m_shape_rotation = 0.0f;
	m_shape_size = 1.0f;
	m_shape_translation = vec2(0.0f);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A0::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Shape Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		// Retrieve red color component from slider and store in the first element of
		// m_shape_color.
		ImGui::SliderFloat("Red Channel", &m_shape_color.r, 0.0f, 1.0f);

		// Add more gui elements here here ...
		ImGui::SliderFloat("Green Channel", &m_shape_color.g, 0.0f, 1.0f);
		ImGui::SliderFloat("Blue Channel", &m_shape_color.b, 0.0f, 1.0f);
		ImGui::SliderFloat("Rotation Channel", &m_shape_rotation, 0.0f, 2*M_PI);


		if( ImGui::Button( "Reset" ) ) {
			resetShape();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A0::draw()
{
	glBindVertexArray(m_vao_triangle);

	m_shader.enable();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	m_shader.disable();


	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A0::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A0::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A0::mouseMoveEvent(double xPos, double yPos) {
	bool eventHandled(false);

	// Fill in with event handling code...

	// Translate current mouse position to GL coordinates:
	m_mouse_GL_coordinate = vec2 (
			(2.0f * xPos) / m_windowWidth - 1.0f,
			1.0f - ( (2.0f * yPos) / m_windowHeight)
	);

	if (m_mouseButtonActive) {
		m_shape_translation = m_mouse_GL_coordinate;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A0::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...

	if (actions == GLFW_PRESS) {
		if (!ImGui::IsMouseHoveringAnyWindow()) {
			m_mouseButtonActive = true;
			m_shape_translation = m_mouse_GL_coordinate;
		}
	}

	if (actions == GLFW_RELEASE) {
		m_mouseButtonActive = false;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A0::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A0::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A0::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);
	constexpr float increment = 1.0f;
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_EQUAL) {
			cout << "+ key pressed" << endl;

			m_shape_size = m_shape_size + increment;
	
			eventHandled = true;
		}
		if (key == GLFW_KEY_MINUS) {
			cout << "- key pressed" << endl;

			m_shape_size = m_shape_size - increment;

			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {

			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			resetShape();
			eventHandled = true;
		}
	}


	return eventHandled;
}
