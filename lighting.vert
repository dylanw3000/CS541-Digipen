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

out vec3 normalVec, lightVec, eyeVec, tanVec;
out vec2 texCoord;
out vec4 shadowCoord;

uniform vec3 lightPos;
// uniform int mode;
uniform mat4 ShadowMatrix;

void LightingVertex(vec3 eye)
{      
    // gl_Position = WorldProj*WorldView*ModelTr*vertex;

    shadowCoord = ShadowMatrix*ModelTr*vertex;
    
    vec3 worldPos = (ModelTr*vertex).xyz;

    normalVec = vertexNormal*mat3(NormalTr); 
    lightVec = lightPos - worldPos;

    texCoord = vertexTexture; 

    // vec3 eyePos = (WorldInverse*vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    eyeVec = eye - worldPos;

    tanVec = mat3(ModelTr)*vertexTangent;
}
