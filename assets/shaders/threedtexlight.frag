#version 150

uniform	sampler2D	albedo;
uniform vec4 light;
uniform vec4 lightcolor;

in	vec3	ex_Position;
in	vec3	ex_Normal;
in	vec2	ex_Texcoord;

out	vec4	out_Color;

void main( void ) {
	vec3 normal = normalize( ex_Normal );
	vec3 lightdir = light.xyz - ex_Position;
	float lightlen = length( lightdir );
	float atten = lightcolor.a / ( 1.0 + lightlen * lightlen * light.w );
	
	float ndotl = max( dot( lightdir / lightlen, normal ), 0.0 );

	vec4 texcolor = texture2D( albedo, ex_Texcoord );
	
	out_Color = atten * ( ndotl ) * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
}
