#version 120
attribute vec3 aVertexPosition;
varying float t_varying;
uniform vec3 aVector;
uniform float aFloat;
uniform float t_uniform;

void main()
{
	t_varying = t_uniform;
	gl_Position = vec4(aVertexPosition*sin(t_uniform), 1.0);
}

