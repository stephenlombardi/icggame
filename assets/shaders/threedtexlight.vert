#version 150

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texcoord;

out vec3 ex_Position;
out vec3 ex_Normal;
out vec2 ex_Texcoord;

void main( void ) {
	ex_Position = vec3(model * vec4(in_Position, 1.0));
	gl_Position = projection * view * model * vec4(in_Position, 1.0);
	ex_Normal = vec3(model * vec4(in_Normal, 0.0));
	ex_Texcoord = in_Texcoord;
}
