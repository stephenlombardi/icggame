#version 150

uniform	sampler2D	albedo;
uniform vec4 light;
uniform vec4 lightcolor;
uniform sampler2D shadowmap;

in	vec3	ex_Position;
in	vec3	ex_Normal;
in	vec2	ex_Texcoord;
in	vec4	ex_Shadow;

out	vec4	out_Color;

void main( void ) {
	vec3 normal = normalize( ex_Normal );
	vec3 lightdir = light.xyz - ex_Position;
	float lightlen = length( lightdir );
	float atten = lightcolor.a / ( 1.0 + lightlen * lightlen * light.w );
	
	float ndotl = max( dot( lightdir / lightlen, normal ), 0.0 );

	vec4 texcolor = texture2D( albedo, ex_Texcoord );
	
	vec4 shadow = ex_Shadow / ex_Shadow.w;
	float lightdist = texture2D( shadowmap, shadow.st ).z;
	float shadowterm = 0.0;
	if( shadow.x >= 0.0 && shadow.x <= 1.0 && shadow.y >= 0.0 && shadow.y <= 1.0 && ex_Shadow.z >= 0.0 && lightdist > shadow.z - 0.0000 ) {
		float r = 1.0 - pow( min( length( shadow.xy * 2.0 - 1.0 ), 1.0 ), 16.0 );
		shadowterm = r;
	}
	
	//out_Color = shadow.x > -1.0 && shadow.x < 1.0 && shadow.y > -1.0 && shadow.y < 1.0 && ex_Shadow.z > 0.0 ? vec4( vec3( pow( lightdist, 256.0 ) ), 1.0 ) : vec4( 0.0, 0.0, 0.0, 1.0 );
	out_Color = shadowterm * atten * ndotl * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
	//out_Color = vec4( vec3( shadowterm ), 1.0 );
	//out_Color = vec4( pow( shadow.z, 32.0 ) );
	//out_Color = vec4( vec3( ex_Shadow.x / 1024.0 ), 1.0 );
	//out_Color = vec4( vec3( 0.25 ), 1.0 );
}
