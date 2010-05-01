#version 150

uniform sampler2D albedotex;
uniform sampler2D normaltex;
uniform sampler2D depthtex;

uniform mat4 viewprojInv;

uniform vec4 lightpos;
uniform vec4 lightcolor;
uniform mat4 lightmatrix;

uniform sampler2D shadowmap;

out vec4 out_Color;

void main( void ) {
	vec2 normFragCoord = gl_FragCoord.xy / 768.0;
	
	float projdepth = texture2D( depthtex, normFragCoord ).r;
	vec4 worldpos = viewprojInv * vec4( normFragCoord.x * 2.0 - 1.0, normFragCoord.y * 2.0 - 1.0, projdepth * 2.0 - 1.0, 1.0 );
	worldpos /= worldpos.w;
	
	vec4 ex_Shadow = lightmatrix * worldpos;
	vec4 shadow = ex_Shadow / ex_Shadow.w;
	float lightdist = texture2D( shadowmap, shadow.st ).z;
	float shadowterm = 0.0;
	if( shadow.x >= 0.0 && shadow.x <= 1.0 && shadow.y >= 0.0 && shadow.y <= 1.0 && ex_Shadow.z >= 0.0 && lightdist > shadow.z - 0.0000 ) {
		float r = 1.0 - pow( min( length( shadow.xy * 2.0 - 1.0 ), 1.0 ), 16.0 );
		shadowterm = r;
		//shadowterm = 1.0;
	}
	
	vec3 lightdir = lightpos.xyz - worldpos.xyz;
	float lightlen = length( lightdir );
	float atten = lightcolor.a / ( 1.0 + lightlen * lightlen * lightpos.w );
	
	vec3 normal = texture2D( normaltex, normFragCoord ).xyz * 2.0 - 1.0;
	float ndotl = max( dot( lightdir / lightlen, normal ), 0.0 );

	vec4 texcolor = texture2D( albedotex, normFragCoord );
	
	//out_Color = vec4( normal * 0.5 + 0.5, 1.0 );
	out_Color = shadowterm * atten * ndotl * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
	//out_Color = vec4( 1.0, shadowterm * atten * ndotl * vec2( texcolor.g * lightcolor.g, texcolor.b * lightcolor.b ), 1.0 );
	//out_Color = atten * ndotl * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
	//out_Color = vec4( ndotl );
	gl_FragDepth = projdepth;
}
