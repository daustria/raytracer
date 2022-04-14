#version 330

in vec2 position;

in vec3 colour;

out vec3 f_colour;

uniform mat4 P;                                                                                                                                                                                                  uniform mat4 V;
uniform mat4 M;

void main() {
	gl_Position = P*vec4(position, 0.0, 1.0);

	f_colour = colour;
}
