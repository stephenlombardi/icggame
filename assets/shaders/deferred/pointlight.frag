#version 150

uniform sampler2D albedotex;
uniform sampler2D normaltex;
uniform sampler2D depthtex;

uniform mat4 viewprojInv;

uniform vec4 lightpos;
uniform vec4 lightcolor;

out vec4 out_Color;

void main( void ) {
	vec2 normFragCoord = gl_FragCoord.xy / 768.0;
	
	float projdepth = texture2D( depthtex, normFragCoord ).r;
	vec4 worldpos = viewprojInv * vec4( normFragCoord.x * 2.0 - 1.0, normFragCoord.y * 2.0 - 1.0, projdepth * 2.0 - 1.0, 1.0 );
	worldpos /= worldpos.w;
	
	vec3 lightdir = lightpos.xyz - worldpos.xyz;
	float lightlen = length( lightdir );
	float atten = lightcolor.a / ( 1.0 + lightlen * lightlen * lightpos.w );
	
	vec3 normal = texture2D( normaltex, normFragCoord ).xyz * 2.0 - 1.0;
	float ndotl = max( dot( lightdir / lightlen, normal ), 0.0 );

	vec4 texcolor = texture2D( albedotex, normFragCoord );
	
	out_Color = atten * ndotl * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
	gl_FragDepth = projdepth;
}
