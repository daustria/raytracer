#version 330

in vec3 vcolour;

out vec4 fragColour;

uniform bool picking; // bool to decide if we are doing picking or not
uniform vec3 pickingColour; //false color of object in picking mode, which is in 1-1 correspondence with its mesh ID

void main() {
	if (picking) {
		fragColour = vec4(pickingColour, 1.);
	} else {
		fragColour = vec4(vcolour,1.);
	}
}
