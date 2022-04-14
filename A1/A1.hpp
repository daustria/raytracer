#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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

private:
	static constexpr size_t DIM = 16;

	void initGrid();
	void initCube();
	void initAvatar();
	void initFloor();

	Maze maze;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader; /* Actual program that we attach the shaders to, and link them */
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	
	/* A note on VAOS:
	 *
	 * The process of drawing a triangle, say, requires a lot of work. We have to make a vertex buffer object,
	 * and tell openGL how to interpret our vertex data by calling glEnableVertexArray, enable the attributes we want with
	 * glVertexAttribPointer. 
	 *
	 * If we set up a vertex array object (VAO) first, it records everything we did in those above steps. Next time we want to render our
	 * triangle again, all that setup is saved in the VAO, so we don't have to make those calls again.
	 * */

	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	GLuint m_cube_vao; 
	GLuint m_cube_vbo;

	GLuint m_sphere_vao;
	GLuint m_sphere_vbo;

	GLuint m_floor_vao;
	GLuint m_floor_vbo;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[3];
	float avatar_colour[3];
	float cube_colour[3];
	float floor_colour[3];

	int current_col;

	enum Colour { Cube = 0, Avatar, Floor };

	void assignColour();

	double old_mouse_x = 0;
	double old_mouse_y = 0;

	/* These initial angles are so that when I put the cursor in the middle of the screen, both rotation angles are about 0 */
	float rotation_angle_y = 211; /* How much to rotate the grid by, with respect to the y-axis, in degrees */
	float rotation_angle_x = -27; /* How much to rotate the grid by, with respect to the x-axis, in in degrees */

	const int num_cube_vertices = 30; /* We are representing a cube with triangles, but we don't bother rendering the face hitting the floor */

	// fields for describing the avatar
	const float radius = 0.5f;
	int num_avatar_vertices = 0; //set later, when we describe the avatar

	//check if the keys are being held so we can move the avatar, smoothly
	bool left_key_held = false;
	bool right_key_held = false;
	bool up_key_held = false;
	bool down_key_held = false;
	bool shift_key_held = false;

	//relative to the grid. the original coordinates of the sphere are (-8,0,8)+(0.5,0,0.5)
	float avatar_x = 0.0f; 
	float avatar_z = 0.0f;

	void processAvatarMovement();

	// for processing avatar's movement and handling collision.
	// x,z correspond to the centre of the avatar sphere (after movement) relative to the grid. 
	// returns true if there is a maze block on cell (floor(x), floor(z)) and false otherwise.
	//
	// if shift key is held, then this function always returns true, and the colliding maze block is destroyed
	bool gridHasMazeBlock(float x, float z);

	int block_height = 1;

	float scale_factor = 1.0f;

	void reset();

};
