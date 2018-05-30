#version 430

// Model/view/projection matrix
layout(location = 0) uniform mat4 mvp;
layout(location = 6) uniform float mixFactor_idle = 0.0;
layout(location = 7) uniform float mixFactor_attack = 0.0;
layout(location = 8) uniform float mixFactor_dead = 0.0;
layout(location = 10) uniform vec3 pos_offset = vec3(0, 0, 0);
layout(location = 11) uniform vec3 rotateAxis = vec3(0, 1, 0);
layout(location = 12) uniform float rotateAngle = 0.0;
layout(location = 13) uniform float scaleFactor = 1.0;
layout(location = 14) uniform bool useShadow = false;
layout(location = 15) uniform bool uniColor = false;

// Per-vertex attributes
layout(location = 0) in vec3 pos; // World-space position
layout(location = 1) in vec3 normal; // World-space normal
layout(location = 2) in vec3 pos_idle;
layout(location = 3) in vec3 normal_idle;
layout(location = 4) in vec3 pos_attack; 
layout(location = 5) in vec3 normal_attack; 
layout(location = 6) in vec3 pos_dead; 
layout(location = 7) in vec3 normal_dead; 
layout(location = 8) in vec2 texCoor;
layout(location = 9) in vec3 shadow;
// Data to pass to fragment shader
out vec3 fragPos;
out vec3 fragNormal;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main() {

vec3 pos_current = pos;
	vec3 normal_current = normal;

	mat4 rMatrix = rotationMatrix(rotateAxis, rotateAngle);
	vec4 tmp;
//	tmp = rMatrix * vec4(pos, 1.0);
//	pos_current = tmp.xyz;
//	tmp = rMatrix * vec4(normal, 1.0);
//	normal_current = tmp.xyz;
	
	// animations
	pos_current = mix(pos_current, pos_idle, mixFactor_idle);
	normal_current = mix(normal_current, normal_idle, mixFactor_idle);
	pos_current = mix(pos_current, pos_attack, mixFactor_attack);
	normal_current = mix(normal_current, normal_attack, mixFactor_attack);
	pos_current = mix(pos_current, pos_dead, mixFactor_dead);
	normal_current = mix(normal_current, normal_dead, mixFactor_dead);

	pos_current = mat3(scaleFactor)*pos_current;
	
	tmp = rMatrix * vec4(pos_current, 1.0);
	pos_current = tmp.xyz;
	tmp = rMatrix * vec4(normal_current, 1.0);
	normal_current = tmp.xyz;

	pos_current += pos_offset;


	// Transform 3D position into on-screen position
    gl_Position = mvp * vec4(pos_current, 1.0);



    // Pass position and normal through to fragment shader
    fragPos = pos_current;
    fragNormal = normal_current;
//	// Transform 3D position into on-screen position
//    gl_Position = mvp * vec4(pos, 1.0);
//
//    // Pass position and normal through to fragment shader
//    fragPos = pos;
//    fragNormal = normal;
}