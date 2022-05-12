#version 330

// Model-Space coordinates
in vec3 position;
in vec3 normal;

struct LightSource {
    vec3 position;
    vec3 rgbIntensity;
};
uniform LightSource light;

uniform mat4 ModelView;
uniform mat4 Perspective;

// Remember, this is transpose(inverse(ModelView)).  Normals should be
// transformed using this matrix instead of the ModelView matrix.
uniform mat3 NormalMatrix;

struct Material {
    vec3 kd;
};
uniform Material material;

// Ambient light intensity for each RGB component.
uniform vec3 ambientIntensity;

out vec3 vcolour;

vec3 diffuseLighting(vec3 vertPosition, vec3 vertNormal) {
    // Direction from vertex to light source.
    vec3 l = normalize(light.position - vertPosition);

    float n_dot_l = max(dot(vertNormal, l), 0.0);

    vec3 diffuse;
    diffuse = material.kd * n_dot_l;

    return ambientIntensity*material.kd + light.rgbIntensity*diffuse;
}

void main() {
	vec4 pos4 = vec4(position, 1.0);

	vcolour = diffuseLighting((ModelView * pos4).xyz, normalize(NormalMatrix * normal));
	
	gl_Position = Perspective * ModelView * pos4;
}
