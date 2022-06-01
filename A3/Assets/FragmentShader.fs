#version 330

in vec3 vcolour;
in vec2 texCoords;

out vec4 fragColour;

uniform bool picking; // bool to decide if we are doing picking or not
uniform vec3 pickingColour; //false color of object in picking mode, which is in 1-1 correspondence with its mesh ID

uniform sampler2D ourTexture;

void main() {
	if (picking) {
		fragColour = vec4(pickingColour, 1.);
	} else {
		fragColour = vec4(vcolour,1.) * texture(ourTexture, texCoords);
	}
}
