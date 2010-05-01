#version 150

uniform sampler2D albedotex;
uniform sampler2D normaltex;
uniform sampler2D depthtex;

uniform mat4 viewprojInv;

uniform vec3 lightdir;
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
	//float lightdist = texture2D( shadowmap, shadow.st ).z;
	float shadowterm = 0.0;
	/*if( shadow.x >= 0.0 && shadow.x <= 1.0 && shadow.y >= 0.0 && shadow.y <= 1.0 && ex_Shadow.z >= 0.0 && lightdist > shadow.z - 0.0000 ) {
		shadowterm = 1.0;
	}*/
	for( float offy = -1.5; offy <= 1.5; offy += 1.0 ) {
		for( float offx = -1.5; offx <= 1.5; offx += 1.0 ) {
			shadowterm += shadow.x >= 0.0 && shadow.x <= 1.0 && shadow.y >= 0.0 && shadow.y <= 1.0 && texture2D( shadowmap, shadow.st + vec2( offx, offy ) / 2048.0 ).z > shadow.z ? 1.0 : 0.0;
		}
	}
	shadowterm /= 16.0;
	
	vec3 normal = texture2D( normaltex, normFragCoord ).xyz * 2.0 - 1.0;
	float ndotl = max( -dot( lightdir, normal ), 0.0 );

	vec4 texcolor = texture2D( albedotex, normFragCoord );
	
	out_Color = shadowterm * ndotl * vec4( texcolor.r * lightcolor.r, texcolor.g * lightcolor.g, texcolor.b * lightcolor.b, 1.0 );
	gl_FragDepth = projdepth;
}
