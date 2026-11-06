#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;  // pozitia in spatiul lumina pt umbre

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix;	// matricea de transformare a umbrelor

void main() 
{
	// pozitie world space
	fPosition = vec3(model * vec4(vPosition, 1.0f));

	// normala in world space
	fNormal = normalize(normalMatrix * vNormal);

	fTexCoords = vTexCoords;

	// pozitia fragmentului vazut de lumina
	fragPosLightSpace = lightSpaceTrMatrix * vec4(fPosition, 1.0f);

	gl_Position = projection * view * vec4(fPosition, 1.0f);

}
