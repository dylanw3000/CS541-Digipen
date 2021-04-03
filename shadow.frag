/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h

in vec4 position;
uniform int mode;

void main()
{
    gl_FragData[0].x = position.w / 150.0;
    gl_FragData[0].y = pow(position.w / 150.0, 2);
    gl_FragData[0].z = pow(position.w / 150.0, 3);
    gl_FragData[0].w = pow(position.w / 150.0, 4);

    if(mode == 2){
        gl_FragData[0] = position;
    }
}
