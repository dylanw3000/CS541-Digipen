/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330
in vec3 eyeVec;
out vec4 FragColor;

void LightingFrag();
void main()
{
    LightingFrag();
}