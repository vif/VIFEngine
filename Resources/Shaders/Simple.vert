#version 400
uniform Data
{
	mat4 mvp;
} data;

in vec4 position;
in vec4 colour;

out vec4 out_position;
out vec4 out_colour;

void main()
{
	gl_Position = data.mvp * position;
	out_colour = colour;
}