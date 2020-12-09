/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr;
uniform bool reflective;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec;
out vec2 texCoord;
out vec4 shadowCoord;

uniform vec3 lightPos;
// uniform int mode;
uniform mat4 ShadowMatrix;

void LightingVertex(vec3);
void main()
{      
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    vec3 eye = (WorldInverse*vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    LightingVertex(eye);
    // LightingVertex();
}
