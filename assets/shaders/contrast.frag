#version 150

uniform sampler2D texture;

in vec2 ex_Texcoord;

out vec4 out_Color;

void main( void ) {
	vec4 tex = texture2D( texture, ex_Texcoord );
	const float p = 8.0;
	out_Color = vec4( pow( tex.r, p ), pow( tex.g, p ), pow( tex.b, p ), pow( tex.a, p ) );
}
