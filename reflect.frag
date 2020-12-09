/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h

// in vec4 position;

void LightingFrag();
void main()
{
    LightingFrag();
    // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    // gl_FragData[0] = position;
    // gl_FragData[0] = vec4(position.w/100);
}
