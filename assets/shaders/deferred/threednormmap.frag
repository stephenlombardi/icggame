#version 150

uniform	sampler2D albedo;
uniform sampler2D normalmap;

in vec3 ex_Normal;
in vec2 ex_Texcoord;
in vec3 ex_Tangent;
in vec3 ex_Bitangent;

out vec4 out_Data[ 2 ];

void main( void ) {
	out_Data[ 0 ] = texture2D( albedo, ex_Texcoord );
	mat3 tbn = mat3( normalize( ex_Tangent ), normalize( ex_Bitangent ), normalize( ex_Normal ) );
	vec4 tangentnormal = texture2D( normalmap, ex_Texcoord ) * 2.0 - 1.0;
	out_Data[ 1 ] = vec4( ( tbn * tangentnormal.xyz ) * 0.5 + 0.5, 1.0 );
	//out_Data[ 1 ] = vec4( normalize( ex_Normal ) * 0.5 + 0.5, 1.0 );
	//out_Data[ 1 ] = vec4( tangentnormal.xyz * 0.5 + 0.5, 1.0 );
}
