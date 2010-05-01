#version 150

uniform sampler2D texture;

in vec2 ex_Texcoord;

out vec4 out_Color;

void main( void ) {
	/*vec4 tex0 = 0.25 * texture2D( texture, ex_Texcoord );
	vec4 tex1 = 0.125 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 768.0, 0.0 ) );
	vec4 tex2 = 0.125 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 768.0, 0.0 ) );
	vec4 tex3 = 0.125 * texture2D( texture, ex_Texcoord + vec2( 0.0, 1.0 / 768.0 ) );
	vec4 tex4 = 0.125 * texture2D( texture, ex_Texcoord + vec2( 0.0, -1.0 / 768.0 ) );
	vec4 tex5 = 0.0625 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 768.0, 1.0 / 768.0 ) );
	vec4 tex6 = 0.0625 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 768.0, 1.0 / 768.0 ) );
	vec4 tex7 = 0.0625 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 768.0, -1.0 / 768.0 ) );
	vec4 tex8 = 0.0625 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 768.0, -1.0 / 768.0 ) );
	out_Color = 2.0 * ( tex0 + tex1 + tex2 + tex3 + tex4 + tex5 + tex6 + tex7 + tex8 );*/
	
	/*const float kernel[ 49 ] = float[ 49 ]( 0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067,
											0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292,
											0.00019117,	0.00655965,	0.05472157,	0.11098164,	0.05472157,	0.00655965,	0.00019117,
											0.00038771,	0.01330373,	0.11098164,	0.22508352,	0.11098164, 0.01330373,	0.00038771,
											0.00019117,	0.00655965,	0.05472157,	0.11098164,	0.05472157, 0.00655965,	0.00019117,
											0.00002292,	0.00078633,	0.00655965,	0.01330373,	0.00655965, 0.00078633,	0.00002292,
											0.00000067,	0.00002292,	0.00019117,	0.00038771,	0.00019117, 0.00002292,	0.00000067 );*/
	
	/*const float kernel[ 9 ] = float[ 9 ]( 1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
										2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
										1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0 );*/
	
	/*vec4 tex = vec4( 0.0 );
	for( int y = -6; y <= 6; y++ ) {
		for( int x = 0; x <= 0; x++ ) {
			tex += texture2D( texture, ex_Texcoord + vec2( x / 768.0, y / 768.0 ) ) / 13.0;
		}
	}
	out_Color = tex * 1.5;*/
	
	
	/*vec4 tex = 0.120985 * texture2D( texture, ex_Texcoord + vec2( 0.0, -2.0 / 768.0 ) ) +
				0.176033 * texture2D( texture, ex_Texcoord + vec2( 0.0, -1.0 / 768.0 ) ) +
				0.199471 * texture2D( texture, ex_Texcoord + vec2( 0.0, 0.0 / 768.0 ) ) +
				0.176033 * texture2D( texture, ex_Texcoord + vec2( 0.0, 1.0 / 768.0 ) ) +
				0.120985 * texture2D( texture, ex_Texcoord + vec2( 0.0, 2.0 / 768.0 ) );*/
				
	vec4 tex = 0.0456623 * texture2D( texture, ex_Texcoord + vec2( 0.0, -5.0 / 768.0 ) ) +
				0.0604927 * texture2D( texture, ex_Texcoord + vec2( 0.0, -4.0 / 768.0 ) ) +
				0.0752844 * texture2D( texture, ex_Texcoord + vec2( 0.0, -3.0 / 768.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( 0.0, -2.0 / 768.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( 0.0, -1.0 / 768.0 ) ) +
				0.0997356 * texture2D( texture, ex_Texcoord + vec2( 0.0, 0.0 / 768.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( 0.0, 1.0 / 768.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( 0.0, 2.0 / 768.0 ) ) +
				0.0752844 * texture2D( texture, ex_Texcoord + vec2( 0.0, 3.0 / 768.0 ) ) +
				0.0604927 * texture2D( texture, ex_Texcoord + vec2( 0.0, 4.0 / 768.0 ) ) +
				0.0456623 * texture2D( texture, ex_Texcoord + vec2( 0.0, 5.0 / 768.0 ) );
	out_Color = tex * 2.0;
	//out_Color = vec4( tex.r * 3.5, 0.0, 0.0, 1.0 );
	//out_Color = vec4( 1.0 );
}
