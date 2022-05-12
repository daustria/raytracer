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
    vec3 ks;
    float shininess;
};
uniform Material material;

// diffuse.vert
out vec4 colour;

void main ( ) {
        vec4 pos4 = vec4(position, 1.0);
	vec3 pos_EC = vec3 ( ModelView * pos4 );
	vec3 normal_EC = normalize ( NormalMatrix * normal ) ;
	vec3 light_EC = normalize ( light.position.xyz - pos_EC );
	float albedo = dot ( normal_EC , light_EC ) ;
	vec4 diffusep = light.rgbIntensity * material.kd;
	colour = albedo * diffusep;
	gl_Position = Perspective * ModelView * vec4(position, 1.0);
}
