#version 150

uniform sampler2D albedotex;
uniform sampler2D normaltex;
uniform sampler2D depthtex;

out vec4 out_Color;

void main( void ) {
	vec2 normFragCoord = gl_FragCoord.xy / 768.0;
	
	vec4 texcolor = texture2D( albedotex, normFragCoord );
	float projdepth = texture2D( depthtex, normFragCoord ).r;
	
	out_Color = 0.1 * texcolor;
	gl_FragDepth = projdepth;
}
