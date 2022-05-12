#version 330

in vec3 vcolour;

out vec4 fragColour;

void main() {
	fragColour = vec4(vcolour,1.);
}
