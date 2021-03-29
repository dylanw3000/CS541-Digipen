
/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 ModelTr, NormalTr, WorldProj,  WorldView, WorldInverse, ShadowMatrix;
uniform vec3 lightPos, eyePos;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

void main()
{
	gl_Position=WorldProj*WorldView*ModelTr*vertex;
}
