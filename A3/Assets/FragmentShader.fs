#version 330

in vec3 vcolour;
in vec2 texCoords;

out vec4 fragColour;

uniform bool picking; // bool to decide if we are doing picking or not
uniform vec3 pickingColour; //false color of object in picking mode, which is in 1-1 correspondence with its mesh ID

uniform sampler2D face; // texture 1
uniform sampler2D suit; // texture 2

// How we pick which texture to use
// 0 for face, 1 for suit, anything else for no texture
uniform int textureNumber = 0; 

void main() {
	if (picking) {
		fragColour = vec4(pickingColour, 1.);
	} else {
		// Not in picking mode, so decide what texture we want to use 
		if (textureNumber == 0) {
			fragColour = vec4(vcolour,1.) * texture(face, texCoords);
		} else if (textureNumber == 1) {
			fragColour = vec4(vcolour,1.) * texture(suit, texCoords);
		} else {
			fragColour = vec4(vcolour,1.);
		}
	}
}
