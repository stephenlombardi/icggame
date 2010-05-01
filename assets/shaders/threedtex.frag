#version 140

uniform sampler2D texture;
uniform float texscale;

in vec2 ex_Texcoord;

out vec4 out_Color;

void main( void ) {
	out_Color = texture2D( texture, ex_Texcoord * texscale );
}
