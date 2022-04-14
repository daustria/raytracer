#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#define NDEBUG
using namespace glm;
using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), maze(DIM)
{
	colour[0] = 0.9f;
	colour[1] = 0.1f;
	colour[2] = 0.8f;

	avatar_colour[0] = 0.0f;
	avatar_colour[1] = 0.2f;
	avatar_colour[2] = 0.3f;

	cube_colour[0] = 0.9f;
	cube_colour[1] = 0.1f;
	cube_colour[2] = 0.8f;

	floor_colour[0] = 0.3f;
	floor_colour[1] = 0.5f;
	floor_colour[2] = 0.7f;

	maze.digMaze();
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	

	// DELETE FROM HERE...
	maze.printMaze();
	// ...TO HERE
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();
	initAvatar();
	initFloor();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	
	/* glm::lookAt(eye, center, up)
	 *
	 * eye: position of camera 
	 * center: where the camera is looking at
	 * up: the worlds 'upward' direction, in this case the positive y-axis 
	 */
	view = glm::lookAt( 

		//glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ), /* (0, 32/sqrt(2), 18/sqrt(2)) */
		glm::vec3( 0.0f, scale_factor*2.*float(DIM)*2.0*M_SQRT1_2, scale_factor*float(DIM)*2.0*M_SQRT1_2 ), 
		//glm::vec3( 0.0f, (float) 64*M_SQRT1_2, (float) 32*M_SQRT1_2 ), /* same thing as the previous line */
		//glm::vec3( 0.0f, 1.0f, 0.0f ), /* shows nothing because all the thin lines are parallel to me */
		//glm::vec3( 0.0f, 1.0f, 1.0f ), /* shows the grid after putting the camera/eye at a little bit of an angle */
		//glm::vec3(0, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		//glm::vec3( 0.0f, (float) 64*M_SQRT1_2, (float) 32*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ));

	/* FOV is pi/6 radians. second is aspect ratio, which is width/height. near and far planes are 1.0f and 1000.0f, which contains
	 * the frustum (the visible space) */
	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
} 
void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;

	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao ); 
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo ); /* m_grid_vbo is just an ID to a buffer. the memory of the actual buffer is managed by openGL on the graphics card */
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo ); /* Makes this the 'active' buffer to use */
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float), /* Uploads the actual data to the buffer */
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" ); /* We want to configure the position attribute, so get the location of position in the vertex shader */
	glEnableVertexAttribArray( posAttrib ); 
	/* 4th parameter is 0, so openGL will figure out the space (in bytes) between each vertex in memory */
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube()
{
	float vertices[] = {
		/* Lower Face (y=0) */
		//-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f, // Triangle 1
		//0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, //Triangle 2

		/* Upper face (y=1) */
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, -1.0f, // Triangle 3
		0.0f, 1.0f, 0.0f, -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, //Triangle 4

		/* x = -1 */
		-1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f,
		-1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, -1.0f, 1.0f, -1.0f,

		/* x = 0 */
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, -1.0f,

		/* z = 0 */
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,


		/* the cube seems to render completely without this extra face */
		/* z = -1 */
		0.0f, 0.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f
	};

	glGenVertexArrays( 1, &m_cube_vao ); 
	glBindVertexArray( m_cube_vao );

	glGenBuffers( 1, &m_cube_vbo ); 
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, 3*num_cube_vertices*sizeof(float), vertices, GL_STATIC_DRAW );

	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	/* Reset state */
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS; //what does this macro do?
}

void addVertex(vector<vector<float>> &vertex_data, vector<float> &dest, int index)
{
	const vector<float> &vertex = vertex_data[index];
	assert(vertex.size() == 3);
	dest.push_back(vertex[0]);
	dest.push_back(vertex[1]);
	dest.push_back(vertex[2]);

#ifndef NDEBUG
	printf("initAvatar() | adding vertex (%f, %f, %f), index:%d\n", vertex[0], vertex[1], vertex[2], index);
#endif
}

void A1::initAvatar()
{
	constexpr int NUM_SECTORS = 15;
	constexpr int NUM_STACKS = 15;

	// for each cell on the sphere, it has 2 triangles
	vector<vector<float>> vertices_tmp;
	vertices_tmp.reserve(NUM_SECTORS*NUM_STACKS*2);// hopefully it's large enough to avoid reallocating

	const float sector_step = 2*M_PI/NUM_SECTORS;
	const float stack_step = M_PI/NUM_STACKS;

	for(int i = 0; i <= NUM_STACKS; ++i)
	{
		float x,y,z, xy; //vertex positions

		float stack_angle = M_PI/2 - i*stack_step; //start from pi/2 and go down to -pi/2
		float sector_angle = 0;

		xy = radius*cosf(stack_angle); // r * cos(phi)
		z = radius*sinf(stack_angle);

		for(int j = 0; j <= NUM_SECTORS; ++j)
		{
			sector_angle = j * sector_step; //start from 0 to 2pi

			x = xy*cosf(sector_angle); //r cos(phi) cos(theta)
			y = xy*sinf(sector_angle); //r cos(phi) sin(theta)

			vertices_tmp.push_back({x,y,z});

#ifndef NDEBUG
			printf("initAvatar() | initializing vertex (%f,%f,%f) stack_angle:%f rad sector_angle:%f rad stack:%d sector:%d\n", x, y, z, stack_angle, sector_angle, i, j);
#endif
		}
	}

	vector<float> sphere_vertices;
	sphere_vertices.reserve( 3*vertices_tmp.size() );

	// warning: this is a bit inefficient..
	// generate CCW index list of sphere triangles
	// k1--k1+1
	// |  / |
	// | /  |
	// k2--k2+1
#ifndef NDEBUG
		printf("initAvatar() | now adding the vertices in the order which they will be triangulated \n");
#endif 
	for(int i = 0; i < NUM_STACKS; ++i)
	{
		int k1 = i * (NUM_SECTORS + 1); // top left vertex of the current stack
		int k2 = k1 + NUM_SECTORS + 1; // top left vertex of the stack below

		for(int j = 0; j < NUM_SECTORS; ++j, ++k1, ++k2) //increment k1 and k2 also, so that they represent vertices of the next sector
		{
#ifndef NDEBUG
			printf("initAvatar() | stack:%d sector:%d\n", i, j);
#endif
			//each cell on the sphere needs 2 triangles, excepting the ones corresponding to the first and last stack
			if(i != 0)
			{
				//put triangle k1-k2-k1+1
				addVertex(vertices_tmp, sphere_vertices, k1);
				addVertex(vertices_tmp, sphere_vertices, k2);
				addVertex(vertices_tmp, sphere_vertices, k1+1);
			}

			if(i != NUM_STACKS - 1)
			{
				//put triangle k1-k2-k1+1
				addVertex(vertices_tmp, sphere_vertices, k1+1);
				addVertex(vertices_tmp, sphere_vertices, k2);
				addVertex(vertices_tmp, sphere_vertices, k2+1);
			}
		}
	}

#ifndef NDEBUG
	printf("initAvatar() | num data points: %d\n", sphere_vertices.size() );
	printf("initAvatar() | num vertices on avatar: %d\n", sphere_vertices.size() / 3);
#endif //NDEBUG

	num_avatar_vertices = sphere_vertices.size() / 3;

	// do the drawing stuff now

	glGenVertexArrays( 1, &m_sphere_vao );
	glBindVertexArray( m_sphere_vao );

	glGenBuffers( 1, &m_sphere_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_sphere_vbo );
	glBufferData( GL_ARRAY_BUFFER, sphere_vertices.size()*sizeof(float), &sphere_vertices[0], GL_STATIC_DRAW ); 

	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr );

	glBindVertexArray( 0 );	
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A1::initFloor()
{
	//the floor will just be two triangles	
	
	float s = (float) DIM;

	float vertices[] = {
		/* First Triangle */
		-s, 0, -s, -s, 0, 0, 0, 0, -s,
		/* Second Triangle */
		0, 0, 0, -s, 0, 0, 0, 0, -s
	};
	// 2 triangles * 3 vertices/triangle * 3 data points / vertex
	int num_elements = 2 * 3 * 3;


	glGenVertexArrays( 1, &m_floor_vao ); 
	glBindVertexArray( m_floor_vao );

	glGenBuffers( 1, &m_floor_vbo ); 
	glBindBuffer( GL_ARRAY_BUFFER, m_floor_vbo );
	glBufferData( GL_ARRAY_BUFFER, num_elements*sizeof(float), vertices, GL_STATIC_DRAW );

	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	/* Reset state */
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS; //what does this macro do?
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		// for the maze cubes
		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		//ImGui::SameLine();
		if( ImGui::RadioButton( "Cubes", &current_col, 0 ) ) {
			// Select this colour.
			current_col = Colour::Cube;
		}
		if( ImGui::RadioButton( "Avatar", &current_col, 0 ) ) {
			current_col = Colour::Avatar;
		}
		if( ImGui::RadioButton( "Grid", &current_col, 0 ) ) {
			current_col = Colour::Floor;
		}

		//based on current_col,
		ImGui::PopID();

		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "x-axis rotation: %f degrees ", fmod(rotation_angle_x, 360.0) );
		ImGui::Text( "y-axis rotation: %f degrees ", fmod(rotation_angle_y, 360.0) );
		ImGui::Text( "mouse_x : %f ", old_mouse_x );
		ImGui::Text( "mouse_y : %f ", old_mouse_y );
		ImGui::Text( "avatar_x : %f ", avatar_x );
		ImGui::Text( "avatar_z : %f ", avatar_z );
		ImGui::Text( "current_col: %d ", current_col );
		ImGui::Text( "block_height: %d ", block_height );
		ImGui::Text( "scale_factor: %f ", scale_factor );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	processAvatarMovement();
	assignColour();
	// Create a global transformation for the model (centre it).
	mat4 W;

	/* Rotate along the y-axis and then x-axis */
	W = glm::rotate( W, glm::radians(rotation_angle_y), glm::vec3(0.0f, 1.0f, 0.0f) );
	W = glm::rotate( W, glm::radians(rotation_angle_x), glm::vec3(1.0f, 0.0f, 0.0f) );
	/* model matrix: matrix to translate the vector by -8,0,-8 (when DIM=16) 
	 * (top left corner of object is originally at (0,0,0) */
	
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	/* For each cube, we should modify W to translate it */

	// send the camera matrix again, because we are including zoom in

	view = glm::lookAt( 
		glm::vec3( 0.0f, scale_factor*2.*float(DIM)*2.0*M_SQRT1_2, scale_factor*float(DIM)*2.0*M_SQRT1_2 ), 
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ));

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao ); // Bind the VAO for the grid before you draw anything 
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );
		
		glBindVertexArray( 0 );

		// Draw the sphere/avatar

		mat4 W_sphere = glm::translate( W, vec3(-0.5f, 0, -0.5f) ); // move the avatar a bit so that its in the centre of a cell
		// the sphere's original coordinates before account for user input, is (-8,0,8) + (-0.5,0,-0.5) = (-8.5,0,-8.5)

		W_sphere = glm::translate( W_sphere, vec3(avatar_x, 0, avatar_z) ); // move the avatar again, according to history of direction-key inputs

		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W_sphere ) ); //send the updated matrix to the shader 

		glBindVertexArray( m_sphere_vao );

		glUniform3f( col_uni, avatar_colour[0], avatar_colour[1],  avatar_colour[2] );

		glDrawArrays( GL_TRIANGLES, 0, num_avatar_vertices );

		glBindVertexArray( 0 );

		// Draw the cubes
		glBindVertexArray( m_cube_vao );

		glUniform3f( col_uni, cube_colour[0], cube_colour[1],  cube_colour[2] );

		/* Translate all cubes initally by (1,0,1), so that we arent drawing cubes on the outer squares of the grid */
		mat4 W_cube = glm::translate( W, vec3( 1.0f, 0.0f, 1.0f ) );

		for(int i = 0; i < DIM; ++i)
		{ 
			for(int j = 0; j < DIM; ++j)
			{
				if (maze.getValue(i,j)) {
					//for now, don't draw the cubes 
					mat4 W_ij = glm::translate( W_cube, vec3( float(i), 0, float(j) ) );
					glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W_ij ) ); //send the updated matrix to the shader 

					glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );

					//draw more blocks above
					for(int k = 1; k < block_height; ++k)
					{
						W_ij = glm::translate( W_ij, vec3(0, 1.0f, 0) );
						glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W_ij ) ); 
						glDrawArrays( GL_TRIANGLES, 0, num_cube_vertices );
					}
				}
			}
		}

		glBindVertexArray( 0 );


		//Draw the floor 
		glBindVertexArray( m_floor_vao );

		glUniform3f( col_uni, floor_colour[0], floor_colour[1], floor_colour[2] );

		mat4 W_floor = glm::translate( W, vec3( float(DIM), 0.0f, float(DIM) ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W_floor ) ); // reset the W matrix 
		glDrawArrays( GL_TRIANGLES, 0, 3*2*2 );
		glBindVertexArray( 0 );
		

	// Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		/* Update the rotation angle */
		rotation_angle_y += xPos - old_mouse_x;
		rotation_angle_x += yPos - old_mouse_y;

		/* Save the mouse coordinates for debugging info */
		old_mouse_x = xPos;
		old_mouse_y = yPos;

		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
	} else {	
		//rotation_factor = 0;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	float increment_factor = 0.1f;
	float scale_adjustment = static_cast<float>(yOffSet)*increment_factor;

	// Zoom in or out.
	if ( scale_adjustment > 0 ) {
		scale_factor = std::min(2.0f, scale_factor + scale_adjustment);
	} else if ( scale_adjustment < 0 ) {
		scale_factor = std::max(0.3f, scale_factor + scale_adjustment);
	}

	return eventHandled;
}
//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {

		// Respond to some key events.
		if ( key == GLFW_KEY_UP ) {
			up_key_held = true;
		}
		if ( key == GLFW_KEY_DOWN ) {
			down_key_held = true;
		}
		if ( key == GLFW_KEY_RIGHT ) {
			right_key_held = true;
		}
		if ( key == GLFW_KEY_LEFT ) {
			left_key_held = true;
		}
		if ( key == GLFW_KEY_LEFT_SHIFT ) {
			shift_key_held = true;
		}
		if ( key == GLFW_KEY_R ) {
			reset();
		}
		if ( key == GLFW_KEY_SPACE ) {
			//treat 3 as the maximum block height
			//not bothering to define it as a variable since i dont think we'll need it anywhere else

			block_height = std::min(3, block_height + 1);
		}
		if ( key == GLFW_KEY_BACKSPACE ) {
			block_height = std::max(block_height - 1, 1);
		}
	}

	if(action == GLFW_RELEASE ) {
		if ( key == GLFW_KEY_UP ) {
			up_key_held = false;
		}
		if ( key == GLFW_KEY_DOWN ) {
			down_key_held = false;
		}
		if ( key == GLFW_KEY_RIGHT ) {
			right_key_held = false;
		}
		if ( key == GLFW_KEY_LEFT ) {
			left_key_held = false;
		}
		if ( key == GLFW_KEY_LEFT_SHIFT ) {
			shift_key_held = false;
		}

	}

	return eventHandled;
}


void A1::processAvatarMovement()
{
	float increment = 0.03f;
	float boundary = increment + radius;
	float relaxed_boundary = boundary/2;
		
	float new_x = avatar_x;
	float new_z = avatar_z;

	// this is probaly the ugliest collision code, but making good collision code isnt really the focus of this assignment anyway
	
	if (up_key_held) {
		//before moving up, check if there are blocks above us that we might bump into. 
		//do something similar for the other cases
		
		if(!gridHasMazeBlock(avatar_x, avatar_z - boundary) 
			&& !gridHasMazeBlock(avatar_x + relaxed_boundary, avatar_z - boundary) 
			&& !gridHasMazeBlock(avatar_x - relaxed_boundary, avatar_z - boundary))
			avatar_z -= increment;
	}
	if (down_key_held) {
		//do something similar for the other cases
		if(!gridHasMazeBlock(avatar_x, avatar_z + boundary) 
			&& !gridHasMazeBlock(avatar_x + relaxed_boundary, avatar_z + boundary) 
			&& !gridHasMazeBlock(avatar_x - relaxed_boundary, avatar_z + boundary))
			avatar_z += increment;
	}

	if (right_key_held) {
		if(!gridHasMazeBlock(avatar_x + boundary, avatar_z) 
			&& !gridHasMazeBlock(avatar_x + boundary, avatar_z + relaxed_boundary) 
			&& !gridHasMazeBlock(avatar_x + boundary, avatar_z - relaxed_boundary))
			avatar_x += increment;
	}
	if (left_key_held) {
		if(!gridHasMazeBlock(avatar_x - boundary, avatar_z) 
			&& !gridHasMazeBlock(avatar_x - boundary, avatar_z + relaxed_boundary) 
			&& !gridHasMazeBlock(avatar_x - boundary, avatar_z - relaxed_boundary))
			avatar_x -= increment;
	}

	//call the top left corner of the grid (0,0). the center of the sphere is in cell (0.5f, 0.5f).
	//get new (x,z) coordinates of the sphere, after incrementing.
	//figure out which cell the center of the is in of the grid. the top left cell we will call (0,0).
	//figure out if there is a block there.	

}

bool A1::gridHasMazeBlock(float x, float z)
{
	//(x,z) are some coordinates in the grid, where the top left corner of the grid is considered (0,0),
	// positive x axis is downwards, and positive z-axis is right

	//get top left corner if the cell we are in	
	// add a half to the coordinates, since the centre of the sphere is initially in (0.5,0.5) 
	double grid_cell_x = floor(x+0.5f); 
	double grid_cell_z = floor(z+0.5f);

	//is there a maze block drawn at (grid_cell_x, grid_cell_z)?
	
	//get the corresponding maze coordinates
	int maze_x = grid_cell_x - 1;
	int maze_z = grid_cell_z - 1;

	// make sure x and z are in [0,DIM] before doing anything	
	if ( 0 <= maze_x && maze_x < DIM && 0 <= maze_z && maze_z < DIM ) {
		bool ret = maze.getValue(maze_x, maze_z);
#ifndef NDEBUG
		if (ret) printf("gridHasMazeBlock(%f,%f) | cell coordinates(%d, %d) has maze block:%d\n", x, z, maze_x+1, maze_z + 1, ret);
#endif

		//if there is a maze block but we are holding shift,
		//just destroy the maze block and return true
		if ( ret && shift_key_held ) {
			maze.setValue(maze_x, maze_z, 0);
			return true;
		}

		return ret;
	}

	return false;
}

void A1::assignColour()
{
	switch(current_col)
	{
		case Colour::Cube:
			cube_colour[0] = colour[0];
			cube_colour[1] = colour[1];
			cube_colour[2] = colour[2];
			break;
		case Colour::Avatar:
			avatar_colour[0] = colour[0];
			avatar_colour[1] = colour[1];
			avatar_colour[2] = colour[2];
			break;
		case Colour::Floor:
			floor_colour[0] = colour[0];
			floor_colour[1] = colour[1];
			floor_colour[2] = colour[2];
			break;
		default:
#ifndef NDEBUG
			printf("assignColour(%d) | warning:default statement reached\n", c);
#endif
			break;

	}
}

void A1::reset()
{
	maze.reset();
	maze.digMaze();

	avatar_x = 0.0f;
	avatar_z = 0.0f;

	rotation_angle_y = 211; /* How much to rotate the grid by, with respect to the y-axis, in degrees */
	rotation_angle_x = -27; /* How much to rotate the grid by, with respect to the x-axis, in in degrees */
	old_mouse_x = 0;
	old_mouse_y = 0;

	block_height = 1;

	current_col = 0;

	colour[0] = 0.7f;
	colour[1] = 0.1f;
	colour[2] = 0.2f;

	avatar_colour[0] = 0.0f;
	avatar_colour[1] = 0.2f;
	avatar_colour[2] = 0.3f;

	cube_colour[0] = 0.8f;
	cube_colour[1] = 0.0f;
	cube_colour[2] = 0.9f;

	floor_colour[0] = 0.3f;
	floor_colour[1] = 0.5f;
	floor_colour[2] = 0.7f;
}
