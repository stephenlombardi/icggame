#version 150

uniform sampler2D albedo;

in vec3 ex_Normal;
in vec2 ex_Texcoord;

out vec4 out_Data[ 2 ];

void main( void ) {
	out_Data[ 0 ] = texture2D( albedo, ex_Texcoord );
	out_Data[ 1 ] = vec4( normalize( ex_Normal ) * 0.5 + 0.5, 1.0 );
}
