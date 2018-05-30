#version 430

// Global variables for lighting calculations
layout(location = 1) uniform vec3 viewPos;
layout(location = 2) uniform sampler2D texShadow;
layout(location = 3) uniform float time;
layout(location = 4) uniform mat4 lightMVP;
layout(location = 5) uniform vec3 lightPos = vec3(3,3,3);
layout(location = 9) uniform sampler2D tex;
layout(location = 14) uniform bool useShadow = false; // use precomputed shadow
layout(location = 15) uniform bool uniColor = false;
layout(location = 16) uniform bool onlyWings = false;
layout(location = 17) uniform bool onlyBody = false;
layout(location = 18) uniform float opacity = 1.0;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos;    // World-space position
in vec3 fragNormal; // World-space normal
in vec2 fragTexCoor;
in vec3 fragShadow;

void main() {

	if(onlyWings == true)
	{
		if (!(fragPos.z < 2.25 && abs(fragPos.x) < 0.5 && fragPos.y > 2.5))
			discard;
	}
	if(onlyBody == true)
	{
		if (fragPos.z < 2.22 && abs(fragPos.x) < 0.5 && fragPos.y > 1.1)
			discard;
	}
	// Output the normal as color
	const vec3 lightDir = normalize(lightPos - fragPos);

	float diffuse = max(dot(fragNormal, lightDir), 0.0);
	vec3 reflectVec = reflect(fragNormal, -lightDir);
	reflectVec = normalize(reflectVec);
	float specular = max(dot(reflectVec, normalize(viewPos - fragPos)), 0.0);

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

	
	vec2 poissonDisk[16] =vec2[](
			vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
			vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464), vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
			vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
			vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790));

	// for shadow
	vec4 fragLightCoord = lightMVP * vec4(fragPos, 1.0);
	fragLightCoord.xyz /= fragLightCoord.w;
	fragLightCoord.xyz = fragLightCoord.xyz*0.5 + 0.5;
	float fragLightDepth = fragLightCoord.z;
	vec2 shadowMapCoord = fragLightCoord.xy;
	float shadowMapDepth = texture(texShadow, shadowMapCoord).x;
	float visibility = 1.0;
	float bias = 0.01; // avoid self-shadow
	float shadowMapWidth = 1200, shadowMapHeight = 1200;

	for (int i = 0; i<16; i++) {
		if (texture(texShadow, shadowMapCoord + poissonDisk[i] / 800.0).x  <  fragLightDepth - bias) {
			visibility -= 1.0/16;
		}
	}

//
//	if (fragLightDepth > texture(texShadow,shadowMapCoord).x + bias)
//	{
//		visibility = 0.0;
//	}

//	outColor = vec4(color.xyz, 1.0);
	vec3 phongColor = color.xyz * (diffuse*0.3 + 0.5) + 0.5*pow(specular, 30)*vec3(1,1,1);
	outColor = vec4(phongColor*visibility, opacity);
//    outColor = vec4(color.xyz * (diffuse * 0.5 + 0.5), 1.0);

}