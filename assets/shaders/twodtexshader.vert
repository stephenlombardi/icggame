#version 140

in vec2 in_Position;
in vec2 in_Texcoord;

out vec2 ex_Texcoord;

void main( void ) {
	gl_Position = vec4(in_Position, 0.0, 1.0);
	ex_Texcoord = in_Texcoord;
}
