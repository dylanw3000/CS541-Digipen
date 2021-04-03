
/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 ModelTr, View, Proj;

in vec4 vertex;

out vec4 shadowCoord;

void main()
{
	gl_Position=Proj*View*ModelTr*vertex;
	shadowCoord = gl_Position;
}
