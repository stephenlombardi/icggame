#version 140

uniform vec4 light;
uniform vec4 lightcolor;

in	vec3	ex_Position;
in	vec3	ex_Color;
in	vec3	ex_Normal;

out	vec4	out_Color;

void main( void ) {
	vec3 normal = normalize( ex_Normal );
	vec3 lightdir = light.xyz - ex_Position;
	float lightlen = length( lightdir );
	float atten = lightcolor.a / ( 1.0f + lightlen * lightlen * light.w );
	
	float ndotl = max( dot( lightdir / lightlen, normal ), 0.0 );

	out_Color = vec4( atten * vec3( ex_Color.r * lightcolor.r, ex_Color.g * lightcolor.g, ex_Color.b * lightcolor.b ) * ( ndotl ), 1.0 );
}
