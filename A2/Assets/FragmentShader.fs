#version 330

in vec3 f_colour;

out vec4 fragColour;

void main() {
	fragColour = vec4(f_colour, 1.0);
}
