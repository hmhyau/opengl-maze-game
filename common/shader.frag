#version 120

varying float t_varying;
uniform vec3 Colour_uniform;

void main()
{
	gl_FragColor = vec4(Colour_uniform,1.0);
}
