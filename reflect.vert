/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr;
// uniform mat4 View, Proj, ModelTr;
uniform bool reflective;
uniform float S;

in vec4 vertex;

// out vec4 position;

void LightingVertex(vec3); 
void main()
{      
    /*
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    vec3 eye = (WorldInverse*vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    LightingVertex(eye);
    */

    
    vec3 eye = vec3(0.0, 0.0, 1.5);

    vec4 P = ModelTr*vertex;

    vec4 R = P - vec4(eye, 1.0);
    float a = (R / length(R)).x;
    float b = (R / length(R)).y;
    float c = (R / length(R)).z;
    c *= S;
    // R = R / length(R);
    
    gl_Position = vec4(a/(1+c), b/(1+c), c*length(R)/1000 - 1.0, 1.0);

    LightingVertex(eye);
    
}
