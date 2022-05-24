#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

// There's only two modes so i won't bother making an enum . global variables will do.
static bool position_mode = true;
static bool joint_mode = false;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  m_doPicking(false)
{
	//Stores all the keys we use in the program, and whether they are
	//held or not 
	m_keyHeld = {
		// Application Menu
		{GLFW_KEY_I, 0},
		{GLFW_KEY_O, 0},
		{GLFW_KEY_S, 0},
		{GLFW_KEY_A, 0},
		{GLFW_KEY_Q, 0},
		// Edit Menu
		{GLFW_KEY_U, 0},
		{GLFW_KEY_R, 0},
		// Options Menu
		{GLFW_KEY_C, 0},
		{GLFW_KEY_Z, 0},
		{GLFW_KEY_B, 0},
		{GLFW_KEY_F, 0},
		// Interaction Modes
		{GLFW_KEY_P, 0}, // Position Mode (translate and rotate the puppet)
		{GLFW_KEY_J, 0}, // Joint Mode (rotate the selected joints)
		{GLFW_KEY_M, 0}, // Hide/Toggle Menu
		// Mouse Buttons
		{GLFW_MOUSE_BUTTON_LEFT, 0},
		{GLFW_MOUSE_BUTTON_RIGHT, 0},
		{GLFW_MOUSE_BUTTON_MIDDLE, 0} 
	};

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	//m_light.position = vec3(10.0f, 10.0f, 10.0f);
	//m_light.rgbIntensity = vec3(0.0f); // light
	m_light.position = vec3(0.0f, 0.0f, 0.0f);
	m_light.rgbIntensity = vec3(1.0f, 1.0f, 1.0f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking"); 

		if (location != -1) {
			glUniform1i(location, m_doPicking ? 1 : 0);

		}

		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}

	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...
	uploadCommonSceneUniforms();

	// Translate the root node based on mouse movements
	m_rootNode->translate(m_rootTranslation);
	m_rootTranslation = {0,0,0}; // Reset the translation
	// TODO: Make it so that we can reset to the inital translation. This can be done by keeping track of all translations 
	// and then applying the inverse translation. Or storing all offset translations in another node, near the root node or separately from the tree
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

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
		ImGui::Text( "Interaction Mode: %s", m_mode == InteractionMode::Position ? "Position" : "Joint"); // This assumes there are only two interaction modes

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms.
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & hierarchicalModelMatrix,
		bool do_picking,
		bool is_selected
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");

		// The model matrix for this geometry node will include the hierarchical matrix and the 
		// matrix local to this node. However, we will push a special version of this node's matrix onto
		// the hierarchical matrix stack that ignores scaling done to this node. It will be node.get_transform(),
		// not node.get_transform_local() (the former does not include scaling and the latter does)
		mat4 modelView = viewMatrix * hierarchicalModelMatrix * node.get_transform_local();
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if (do_picking) {
			// get the mesh ID of the node
			const unsigned int node_id = node.m_nodeId;

			// compute the false colour of this node, to be used for picking
			float r = float(node_id & 0xff) / 255.0f; // red value corresponds to the first 8 bits of the id
			float g = float((node_id>>8) & 0xff) / 255.0f; // green value is the next 8 bits
			float b = float((node_id>>16) & 0xff) / 255.0f; // ... and the blue value is the next 8 bits after that

			location = shader.getUniformLocation("pickingColour");

			if (location != -1) {
				glUniform3f( location, r, g, b ); // now upload the colour
			}
		}

		// Question: Should we still do this if we are doing picking?? I'll try disabling this later when the m_doPicking flag is set

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;

		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd;

		if (is_selected) {
			kd = vec3(1.0f, 1.0f, 0);
		} else {
			kd = node.material.kd;
		}

		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );

	renderSceneGraph(*m_rootNode);
	// Reset the joint node rotations
	m_jointRotation = {0,0};


	glDisable( GL_DEPTH_TEST );
	renderArcCircle();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering
	glBindVertexArray(m_vao_meshData);
	
	static bool firstRun = true;

	m_matrixStack.push(root.get_transform());

	processNode(root);

	firstRun = false;

	//No need to push the root transform off the matrix stack, because we're going to empty it anyway
	m_matrixStack.reset();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::processNode(SceneNode &node)
{
	switch(node.m_nodeType)
	{
		case NodeType::GeometryNode:
		{
			// Render the mesh and material associated with the node
			const GeometryNode *geometryNode = static_cast<const GeometryNode *>(&node);

			bool selected = m_selected[node.m_nodeId];

			updateShaderUniforms(m_shader, *geometryNode, m_view, m_matrixStack.active_transform, m_doPicking, selected);

			// For geometry node, get_transform() returns the local transform at the node but with scaling not included.
			// The matrix containing this node's transformations including the scaling is in GeometryNode::get_transform_local()
			m_matrixStack.push(geometryNode->get_transform());
			
			// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
			BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

			//-- Now render the mesh:
			m_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_shader.disable();

			break;
		}
		case NodeType::JointNode:
			// For the Joint node, if the corresponding geometry node(s) is selected, 
			// we rotate it along the x,y axes based on the recent mouse movement on the screen. 

			// This assumes that joint nodes have the corresponding geometry nodes as direct children,
			// or that selected geometry nodes have joint nodes as their direct parent

			// First we look for child nodes that are selected geometry nodes, 

			for (const SceneNode * child : node.children) {
				if (child->m_nodeType == NodeType::GeometryNode) {

					if (m_selected.count(child->m_nodeId) == 0) {
						m_selected[child->m_nodeId] = 0;
					} else if (m_selected[child->m_nodeId]) {
						// Our child is a selected geometry node, so we rotate ourself (a joint node)
						// which will in turn be passed down to our child geometry node
						node.rotate('x', m_jointRotation.x);
						node.rotate('y', m_jointRotation.y);
					}

				}
			}

			// Also don't forget to pass the node's transform along the tree
			m_matrixStack.push(node.get_transform());			
			break;
		case NodeType::SceneNode:
			// For the Scene node, we do nothing but push the local transform
			m_matrixStack.push(node.get_transform());
			break;
		default:
			//We should never be here since there are only three node types
			printf("Default case reached, aborting");
			abort();
	}	

	// After rendering the node we process all its children
	// (make a copy first so we can safely modify the list in the mutual recursion)
	
	std::list<SceneNode *> children_copy = node.children;
	processNodeList(children_copy);
	// We have rendered all its children, so we can pop off the node's local transform
	m_matrixStack.pop();

}
//----------------------------------------------------------------------------------------
void A3::processNodeList(std::list<SceneNode *> &nodes)
{
	if(nodes.empty()) {
		return;
	} 

	SceneNode *first = nodes.front();
	nodes.pop_front();

	processNode(*first);
	processNodeList(nodes);

}
//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	static const double scale_factor(10.0);

	switch(m_mode)
	{
		case InteractionMode::Position:
			if (m_keyHeld[GLFW_MOUSE_BUTTON_LEFT]) {

				// Translate puppet in the xy plane according to mouse's xy offset
				double offset_x = xPos - m_mouse.x;
				double offset_y = m_mouse.y - yPos;

				// Translate the root node based on the offsets
				m_rootTranslation.x = offset_x / scale_factor;
				m_rootTranslation.y = offset_y / scale_factor;

				eventHandled = true;
			}

			if (m_keyHeld[GLFW_MOUSE_BUTTON_MIDDLE]) {
				// Translate puppet in the z-axis according to the mouse's x offset
				double offset = xPos - m_mouse.x;

				m_rootTranslation.z = offset / scale_factor;

				eventHandled = true;
			}
			break;
		case InteractionMode::Joint:

			if (m_keyHeld[GLFW_MOUSE_BUTTON_MIDDLE]) {
				double offset_x = xPos - m_mouse.x;
				double offset_y = m_mouse.y - yPos;

				m_jointRotation.x = offset_x;
				m_jointRotation.y = offset_y;

				eventHandled = true;
			}
			break;
		default:
			printf("%s | Error: Default case reached\n", __func__);
			abort();
	}

	// Reset the mouse coordinates to last known screen coordinates for the next mouse move event
	m_mouse = {xPos, yPos};

	// Fill in with event handling code...
	
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	
	// Picking code --------------------------------------------------------
	if (m_mode == InteractionMode::Joint) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
			m_doPicking = true;

			double xpos, ypos;

			// Note: m_window is a member of CS488Window which we inherited from
			glfwGetCursorPos( m_window, &xpos, &ypos );

			uploadCommonSceneUniforms();

			// Make the background white, and also clear buffers (hopefully no node ID maps to a white background)
			glClearColor(0.85, 0.85, 0.85, 1.0);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			// glClearColor(0.35, 0.35, 0.35, 1.0); Do we need this?

			draw(); // Draw the scene again with our new false colours and white background

			// I don't know if we need these are necesesary
			// glFlush();
			// glFinish();

			// The adjustment of the mouse coordinates are from sample code. I am not sure why the relative measurement of
			// ypos is necessary. Also the frameBuffer coordinates may be different from the window coordinates, and we adjust so that
			// our xpos and ypos are in framebuffer coordinates

			xpos *= double(m_framebufferWidth) / double(m_windowWidth);
			ypos = m_windowHeight - ypos;
			ypos *= double(m_framebufferHeight) / double(m_windowHeight);uploadCommonSceneUniforms();                

			// I am not sure why, but the sample code is set up so that we read the colour from the back buffer. I think it has to do with not
			// wanting the false colours to actually show up on the screen ? 
			glReadBuffer( GL_BACK );
			GLubyte buffer[4] = {0,0,0,0};
			// Actually read the pixel at the mouse location.
			glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
			CHECK_GL_ERRORS;

			// Reassemble the node ID
			unsigned int node_id = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

			m_doPicking = false;

			// Check if node_id is in the selected map. If it is, switch the boolean flag. otherwise
			// we put it in and initialize it to 1 since it was selected
			if (m_selected.count(node_id) > 0) {
				m_selected[node_id] = !m_selected[node_id];
			} else {
				m_selected[node_id] = 1;
			}	

			eventHandled = true;
		}
	}


	if (m_keyHeld.count(button) > 0) {	

		if (actions == GLFW_PRESS) {
			m_keyHeld[button] = true;
		} 

		if (actions == GLFW_RELEASE) {
			m_keyHeld[button] = false;
		} 

		eventHandled = true;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}

		if( key == GLFW_KEY_P ) {
			m_mode = InteractionMode::Position;
			eventHandled = true;
		}

		if( key == GLFW_KEY_J ) {
			m_mode = InteractionMode::Joint;
			eventHandled = true;
		}
	}

	// Fill in with event handling code...	

	return eventHandled;
}
//----------------------------------------------------------------------------------------
void A3::MatrixStack::push(const mat4 &m)
{
	matrices.push(m);
	active_transform = active_transform*m;
}

//----------------------------------------------------------------------------------------
void A3::MatrixStack::pop()
{
	mat4 m_inverse = glm::inverse(matrices.top());

	// 'Undo' the matrix by right multiplying the active transform by the inverse
	active_transform = active_transform * m_inverse;

	// We don't need it anymore
	matrices.pop();
}

//----------------------------------------------------------------------------------------
void A3::MatrixStack::reset()
{
	matrices = stack<mat4>();
	active_transform = mat4(1.0f);
}
