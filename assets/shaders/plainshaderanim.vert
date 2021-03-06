#version 150

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bonematrices[ 20 ];

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texcoord;
in float in_Boneid;

void main( void ) {
	gl_Position = projection * view * model * bonematrices[ int( in_Boneid ) ] * vec4(in_Position, 1.0);
}
