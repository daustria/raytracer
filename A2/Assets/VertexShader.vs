#version 330

in vec2 position;

in vec3 colour;

out vec3 f_colour;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);

	f_colour = colour;
}
