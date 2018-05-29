#version 430

// Global variables for lighting calculations
layout(location = 1) uniform vec3 viewPos;
layout(location = 2) uniform sampler2D texShadow;
layout(location = 3) uniform float time;
layout(location = 4) uniform mat4 lightMVP;
layout(location = 5) uniform vec3 lightPos = vec3(3,3,3);
layout(location = 9) uniform sampler2D tex;
layout(location = 14) uniform bool useShadow = false;
layout(location = 15) uniform bool uniColor = false;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos;    // World-space position
in vec3 fragNormal; // World-space normal
in vec2 fragTexCoor;
in vec3 fragShadow;

void main() {

	// Output the normal as color
	const vec3 lightDir = normalize(lightPos - fragPos);

	float diffuse = max(dot(fragNormal, lightDir), 0.0);
	vec4 color = texture(tex, vec2(fragTexCoor.x, 1.0-fragTexCoor.y));
	
	if (uniColor == true)
	{
		color = vec4(1.0,0.0,0.0,1.0);
	}
	
	if(useShadow == true)
	{
		color.x *= fragShadow.x;
		color.y *= fragShadow.y;
		color.z *= fragShadow.z;
	}

	
//	outColor = vec4(color.xyz, 1.0);
    outColor = vec4(color.xyz * (diffuse * 0.5 + 0.5), 1.0);

}