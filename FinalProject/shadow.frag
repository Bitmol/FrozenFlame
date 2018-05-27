#version 430

// Global variables for lighting calculations
layout(location = 1) uniform vec3 viewPos;
layout(location = 2) uniform float time;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

// Note: There is no output for on-screen color
// 
// Since this is for shadow mapping, we only care about
//   the depth of this fragment, which is written automatically
//   to the bound depth buffer 

void main() 
{
}