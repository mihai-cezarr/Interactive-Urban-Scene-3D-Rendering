#version 410 core

uniform sampler2D diffuseTexture;
in vec2 texCoords;

void main() {
	// don t need any color output, openGL writes automatically in the depth buffer

	// alpha testing for eliminating transparent background from being written in the depthMap
	float alpha = texture(diffuseTexture, texCoords).a;
	if(alpha < 0.1f){
		discard;
	}
}