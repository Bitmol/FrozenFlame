#version 430

// Global variables for lighting calculations
layout(location = 1) uniform vec3 viewPos;
layout(location = 2) uniform sampler2D texShadow;
layout(location = 3) uniform float time;
layout(location = 4) uniform mat4 lightMVP;
layout(location = 5) uniform vec3 lightPos = vec3(3,3,3);
layout(location = 9) uniform sampler2D tex;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos;    // World-space position
in vec3 fragNormal; // World-space normal
in vec2 fragTexCoor;

void main() {

	// Output the normal as color
	const vec3 lightDir = normalize(lightPos - fragPos);

	float diffuse = max(dot(fragNormal, lightDir), 0.0);
	vec4 color = texture(tex, vec2(fragTexCoor.x, 1.0-fragTexCoor.y));
    outColor = vec4(color.xyz * (diffuse * 0.8 + 0.2), 1.0);

}