/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 View, Proj, ModelTr;
uniform bool reflective;

in vec4 vertex;

out vec4 position;

void main()
{      
    gl_Position = Proj*View*ModelTr*vertex;
    position = gl_Position;
}
