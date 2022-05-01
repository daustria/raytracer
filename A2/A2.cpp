#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/ext.hpp>

#include <math.h>
#include <assert.h>

using namespace glm;

std::vector<glm::vec3> A2::m_cubeVertices;
std::vector<int> A2::m_cubeLineIndices; 

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
	m_model(1.0f),
	m_camera(1.0f),
	m_orth_proj(1.0f),
	m_persp(1.0f),
	m_proj(1.0f),
	m_viewport(1.0f)
{

	// scale view 
	m_scaleAdj.initAll(0.5f, 0.1f, 1.0f, 0.0f);

	
	// Eye translation view 	
	m_eyeAdj.initAll(0, 0.1f, 0.5f, -0.5f);
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	initCubeVertices();
	initCubeIndices();
	initMatrices();
}

//----------------------------------------------------------------------------------------
void A2::initCubeVertices()
{
	m_cubeVertices.reserve(8);
	//z =-1
	m_cubeVertices.push_back({-1.0f, -1.0f, -1.0f});
	m_cubeVertices.push_back({1.0f, -1.0f, -1.0f});
	m_cubeVertices.push_back({1.0f, 1.0f, -1.0f});
	m_cubeVertices.push_back({-1.0f, 1.0f, -1.0f});

	//z = 1
	m_cubeVertices.push_back({-1.0f, -1.0f, 1.0f});
	m_cubeVertices.push_back({1.0f, -1.0f, 1.0f});
	m_cubeVertices.push_back({1.0f, 1.0f, 1.0f});
	m_cubeVertices.push_back({-1.0f, 1.0f, 1.0f});
}

//----------------------------------------------------------------------------------------
void A2::initCubeIndices()
{
	m_cubeLineIndices.reserve(24);

	//lines for first face (z=-1)
	m_cubeLineIndices.push_back(0);
	m_cubeLineIndices.push_back(1);

	m_cubeLineIndices.push_back(1);
	m_cubeLineIndices.push_back(2);

	m_cubeLineIndices.push_back(2);
	m_cubeLineIndices.push_back(3);

	m_cubeLineIndices.push_back(3);
	m_cubeLineIndices.push_back(0);

	//lines for second face (z=1)
	m_cubeLineIndices.push_back(4);
	m_cubeLineIndices.push_back(5);

	m_cubeLineIndices.push_back(5);
	m_cubeLineIndices.push_back(6);

	m_cubeLineIndices.push_back(6);
	m_cubeLineIndices.push_back(7);

	m_cubeLineIndices.push_back(7);
	m_cubeLineIndices.push_back(4);

	//lines joining the two faces
	m_cubeLineIndices.push_back(0);
	m_cubeLineIndices.push_back(4);

	m_cubeLineIndices.push_back(1);
	m_cubeLineIndices.push_back(5);

	m_cubeLineIndices.push_back(2);
	m_cubeLineIndices.push_back(6);

	m_cubeLineIndices.push_back(3);
	m_cubeLineIndices.push_back(7);

}

//----------------------------------------------------------------------------------------
void A2::updateCameraMatrix(glm::vec3 eye, glm::vec3 gaze, glm::vec3 up)
{
	// we can derive a basis of R^3 consisting of these vectors, call it {u,v,w}

	// Open question: what is the point of multiplying by -1?
	glm::vec3 w = glm::normalize(gaze) * -1.0f;
	glm::vec3 u = glm::normalize(glm::cross(up, w));
	glm::vec3 v = glm::cross(u,w);

	m_camera[0] = vec4( u.x, u.y, u.z, 0 );
	m_camera[1] = vec4( v.x, v.y, v.z, 0 );
	m_camera[2] = vec4( w.x, w.y, w.z, 0 );
	m_camera[3] = vec4( eye.x, eye.y, eye.z, 1 );

	// before the matrix, notice that the above is a matrix taking the {u,v,w} basis to the 
	// {x,y,z}. to see this, consider the result after applying the map to a point P = (u_0, v_0, w_0, 1) in
	// u,v,w coordinates. howeveer we want the matrix taking the {x,y,z} basis to {u,v,w}, so we take the inverse.
	m_camera = glm::inverse(m_camera);	

}

void A2::updateProjectionMatrix(float r, float t, float n, float f)
{
	//for the view matrix we'll just use an orthographic projection for now
	//page 160 of shirleys book 'fundamentals of computer graphics'

	m_orth_proj[0] = glm::vec4( (float) 1/r, 0, 0, 0 );
	m_orth_proj[1] = glm::vec4( 0, (float) 1/t, 0, 0 );
	m_orth_proj[2] = glm::vec4( 0, 0, (float) 2/(n-f), 0 );
	m_orth_proj[3] = glm::vec4( 0, 0, (float) (n+f)/f-n, 1.0f );


	//keep the perspective matrix to be the identity, for now
	
	//perspective matrix from shirleys book
		
	m_persp[0] = glm::vec4( n, 0, 0, 0 );
	m_persp[1] = glm::vec4( 0, n, 0, 0 );
	m_persp[2] = glm::vec4( 0, 0, n+f, 1.0f );
	m_persp[3] = glm::vec4( 0, 0, -1.0f *n*f, 0 );

	//the perspective matrix maps the perspective view volume to the orthographic view volume.
	//the orthographic view volume is mapped to the canonical view volume using the orthographic projection matrix.
	//so , the perspective projection matrix is just the composition
	
	m_proj = m_orth_proj * m_persp;

}

float applyPlaneFunction(const vec4 &v, const vec4 &plane_normal, const vec4 &plane_point)
{
	assert(v.w != 0);
	return glm::dot(plane_normal, v - plane_point);
}

// clips the line against the plane. modifies a,b so that the line from a to b is 
// the new line after clipping. returns true if we discard both points, false otherwise
bool clipLinePlane(vec4 &a, vec4 &b, const vec4 &plane_normal, const vec4 &plane_point)
{	
	float f_a = applyPlaneFunction(a, plane_normal, plane_point);
	float f_b = applyPlaneFunction(b, plane_normal, plane_point);


	if (signbit(f_a) == signbit(f_b)) {

		// a and b are on the same side of the plane.. so there is no clipping to be done.
		// TODO: if a,b are not in the viewing volume then i should not draw the line. but how do i know
		// if they are in the viewing volume in general? for now, i will see it this way: if f(p) < 0,
		// then f(p) is 'inside' the plane and i will keep the point. otherwise if f(p) > 0 then i discard it.
		if (f_a >= 0) {
			a = vec4(0,0,0,1);
			b = vec4(0,0,0,1);
			return true;
		}
		// they are both inside so there is no clipping to be done
		return false;
	}

	// a,b are on different sides of the plane.. so now i need to decide which how to shorten it. 
	// which is the point that lies outside the plane? again i will decide that if f(a) < 0, i will keep that point.
	// first, we make sure that 'a' is the point we keep (on the 'inside' of the plane) 
		
	//so we want to find when the line AB (defined by equation A + t(B-A) = 0) intersects the plane (N,P-Q) = 0.
	//we do this by plugging the expression for the first equation into the variable P in the second equation. 
	//doing so, we get the following formula for t
	
	float t = f_a / glm::dot(plane_normal, a-b);
	
	//make sure that t is in the interval of [0,1] so that its actually on the line [A,B]
	//assert(0 <= t && t <= 1);
	//Now let B to be the intersection point, instead of the point outside the plane
	vec4 intersection = a + t*(b-a);	

	//if A is on the outside, make it the intersection point
	if (f_a >= 0) {
		a = intersection;
	} else {
		//otherwise B is on the outside, so make B the intersection point
		b = intersection;
	}

	return false;
}

void clipLineSymmetricCube(vec4 &a, vec4 &b, bool print_data)
{

#ifndef NDEBUG
	if(print_data) {
		printf("A2::%s() | before clipping: a {%f,%f,%f} b {%f,%f,%f}\n", __func__, a.x, a.y, a.z, b.x, b.y, b.z);
	}
#endif
	bool clipped = false;
	clipped |= clipLinePlane(a, b, vec4(-1,0,0,1), vec4(-1,0,0,1)); // left face
	clipped |= clipLinePlane(a, b, vec4(1,0,0,1), vec4(1,0,0,1)); //right

	clipped |= clipLinePlane(a, b, vec4(0,1,0,1), vec4(0,1,0,1)); // top
	clipped |= clipLinePlane(a, b, vec4(0,-1,0,1), vec4(0,-1,0,1)); // bottom

	clipped |= clipLinePlane(a, b, vec4(0,0,1,1), vec4(0,0,1,1)); // near
	clipped |= clipLinePlane(a, b, vec4(0,0,-1,1), vec4(0,0,-1,1)); // far

#ifndef NDEBUG
	if(print_data && clipped) {
		printf("A2::%s() | after clipping: a {%f,%f,%f} b {%f,%f,%f}\n", __func__, a.x, a.y, a.z, b.x, b.y, b.z );
	}
#endif

}

void A2::initMatrices()
{
	//translate by 2 units towards the negative z-axis

	m_model[0] = glm::vec4(m_scaleAdj.left(), 0, 0, 0);
	m_model[1] = glm::vec4(0, m_scaleAdj.middle(), 0, 0);
	m_model[2] = glm::vec4(0, 0, m_scaleAdj.right(), 0);
	m_model[3] = glm::vec4(0, 0, -0.5f, 1.0f); // represents a translation of the origin

	updateCameraMatrix(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//cosntruct the matrix for an orthographic projection. 
	//for now let's just use these default values, but i should be using const values 
	
	updateProjectionMatrix(3.0f, 3.0f, -1.0f, -5.0f);

	//Keep the viewport matrix to be the identity for now..
/*
	// the viewport matrix is always constant since the screen dimensions and canonical view volume are constant
	m_viewport[0] = glm::vec4( (float) SCREEN_WIDTH/2, 0, 0, (float) (SCREEN_WIDTH-1)/2 );	
	m_viewport[2] = glm::vec4( 0, 0, 1, 0 );
	m_viewport[3] = glm::vec4( 0, 0, 0, 1 );
*/

#ifndef NDEBUG
	cout << "MODEL:" << endl << m_model << endl;
	cout << "CAMERA:" << endl << m_camera << endl;
	cout << "PROJECTION:" <<  endl << m_proj << endl;
	cout << "VIEWPORT:" << endl << m_viewport << endl;
	cout << endl;
#endif

	m_final = m_viewport * m_proj * m_camera * m_model;
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link(); }

//---------------------------------------------------------------------------------------- Spring 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {
	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{

	/*
	 * Game plan: For a line PQ in model coordinates:
	 *
	 *  -Move to world coordinates
	 *  -Clip against the near plane in world coordinates. only the near plane.
	 *  -Move to projective coordiantes by applying camera and then perspective projection.
	 *  -Clip to the symmetric cube viewing volume, [-1,1]^3 
	 *  -Homogenize the point
	 *  -Move to viewport coordinates by applying the viewport transform (can omit for now since viewport is just identity)
	 *
	 */
	static bool firstRun(true);
	// Place per frame, application logic here ...
	// Call at the beginning of frame, before drawing lines:
	initLineData();

	setLineColour(vec3(1.0f, 0.7f, 0.8f));

	//draw each line	
	for(int i = 0; i+1 < m_cubeLineIndices.size(); i += 2)
	{
		int p_index = m_cubeLineIndices[i];
		int q_index = m_cubeLineIndices[i+1];

		//Get the line in model coordinates
		
		vec4 p_model = vec4{m_cubeVertices[p_index], 1};
		vec4 q_model = vec4{m_cubeVertices[q_index], 1};

#ifndef NDEBUG
		if(firstRun) {
			printf("==============================================================\n");
			printf("A2::%s() | Line in model space: (%f,%f,%f,%f) -- (%f,%f,%f,%f)\n", __func__, 
					p_model.x, p_model.y, p_model.z, p_model.w, q_model.x, q_model.y, q_model.z, q_model.w );
		}
#endif

		//Move to camera coordinates
		mat4 model_to_camera = m_camera*m_model;
		
		vec4 p_cam = model_to_camera* p_model;
		vec4 q_cam = model_to_camera * q_model;

		//Clip the line against the near plane, keep normal pointing outwards
		// TODO: For now this assumes the near plane is at (0,0,-1,1). but we should 
		// have variable near planes. 
		bool discarded = clipLinePlane(p_cam, q_cam, vec4(0,0,1,1), vec4(0,0,-1,1));

		if(firstRun) {
#ifndef NDEBUG
			printf("A2::%s() | Line in camera space: (%f,%f,%f,%f) -- (%f,%f,%f,%f)\n", __func__, 
					p_cam.x, p_cam.y, p_cam.z, p_cam.w, q_cam.x, q_cam.y, q_cam.z, q_cam.w);
#endif
		}


		if (discarded) {
#ifndef NDEBUG
			if(firstRun) {
				printf("A2::%s() | both points in line are clipped, not drawing this line...\n", __func__);
			}
#endif
			continue;
		}
		


		// now do the projective transformation, getting it in homogeneous coordinates	

		vec4 p_proj = m_proj*p_cam;
		vec4 q_proj = m_proj*q_cam;

#ifndef NDEBUG
		if(firstRun) {
			printf("A2::%s() | Line in homogeneous coordinates: [%f:%f:%f:%f] -- [%f:%f:%f:%f]\n", __func__, 
					p_proj.x, p_proj.y, p_proj.z, p_proj.w, q_proj.x, q_proj.y, q_proj.z, q_proj.w );
		}
#endif



		// The projective transformation moves the frustum to the symmetric cube [-1,1]^3. We now clip against this cube.

		clipLineSymmetricCube(p_proj, q_proj, false);

#ifndef NDEBUG
		if(firstRun) {
			printf("A2::%s() | Line after clipping to [-1,1]^3 : [%f:%f:%f:%f] -- [%f:%f:%f:%f]\n", __func__, 
					p_proj.x, p_proj.y, p_proj.z, p_proj.w, q_proj.x, q_proj.y, q_proj.z, q_proj.w);
		}
#endif

		//Divide by the 4th coordinate
		homogenize4thChart(p_proj);
		homogenize4thChart(q_proj);

#ifndef NDEBUG
		if(firstRun) {
			printf("A2::%s() | Line after homogenizing: [%f:%f:%f:%f] -- [%f:%f:%f:%f]\n", __func__, 
					p_proj.x, p_proj.y, p_proj.z, p_proj.w, q_proj.x, q_proj.y, q_proj.z, q_proj.w);
		}
#endif

		drawLine({p_proj.x, p_proj.y}, {q_proj.x, q_proj.y});
	}
	firstRun = false;

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices); // why arent these lines being drawn??
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

void homogenize4thChart(vec4 &v)
{
	assert(v.w != 0.0f);
	v = {v.x/v.w, v.y/v.w, v.z/v.w, 1};
}

//ViewAdjustor-------------------------------------------------------------------------------
ViewAdjustor::ClampedFloat::ClampedFloat(float value, float increment, float max, float min) : 
	value(value), increment(increment), maximum(max), minimum(min) 
{

}

void ViewAdjustor::ClampedFloat::incrementValue(bool positive) 
{
	float new_value = value + (positive ? increment : -1*increment);

	if (new_value <= minimum || new_value >= maximum) {
		return;
	}

	value = new_value;
}

void ViewAdjustor::initLeft(float value, float increment, float max, float min) 
{
	left_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initMiddle(float value, float increment, float max, float min) 
{
	middle_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initRight(float value, float increment, float max, float min) 
{
	right_ = ClampedFloat(value, increment, max, min);
}

void ViewAdjustor::initAll(float value, float increment, float max, float min)
{
	initLeft(value, increment, max, min);	
	initRight(value, increment, max, min);	
	initMiddle(value, increment, max, min);	
}

// Getters 
float ViewAdjustor::left() const
{
	return left_.value;
}

float ViewAdjustor::right() const
{
	return right_.value;
}

float ViewAdjustor::middle() const
{
	return middle_.value;
}

