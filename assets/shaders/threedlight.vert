#version 140

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 in_Position;
in vec3 in_Color;
in vec3 in_Normal;

out vec3 ex_Position;
out vec3 ex_Color;
out vec3 ex_Normal;

void main( void ) {
	ex_Position = vec3(model * vec4(in_Position, 1.0));
	gl_Position = projection * view * vec4(ex_Position,1.0);
	ex_Color = in_Color;
	ex_Normal = vec3(model * vec4(in_Normal, 0.0));
}
