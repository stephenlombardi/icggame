#version 150

uniform sampler2D texture;
uniform float intensity;

in vec2 ex_Texcoord;

out vec4 out_Color;

void main( void ) {
	out_Color = intensity * texture2D( texture, ex_Texcoord );
}
