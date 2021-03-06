#version 150

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bonematrices[ 20 ];
uniform mat4 lightmatrix;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texcoord;
in float in_Boneid;

out vec3 ex_Position;
out vec3 ex_Normal;
out vec2 ex_Texcoord;
out vec4 ex_Shadow;

void main( void ) {
	ex_Position = vec3(model * bonematrices[ int( in_Boneid ) ] * vec4(in_Position, 1.0));
	gl_Position = projection * view * model * bonematrices[ int( in_Boneid ) ] * vec4(in_Position, 1.0);
	ex_Normal = vec3(model * bonematrices[ int( in_Boneid ) ] * vec4(in_Normal, 0.0));
	ex_Texcoord = in_Texcoord;
	ex_Shadow = lightmatrix * model * bonematrices[ int( in_Boneid ) ] * vec4(in_Position, 1.0);
}
