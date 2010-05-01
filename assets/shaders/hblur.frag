#version 150

uniform sampler2D texture;

in vec2 ex_Texcoord;

out vec4 out_Color;

void main( void ) {
	/*vec4 tex = vec4( 0.0 );
	for( int y = 0; y <= 0; y++ ) {
		for( int x = -6; x <= 6; x++ ) {
			tex += texture2D( texture, ex_Texcoord + vec2( x / 192.0, y / 192.0 ) ) / 13.0;
		}
	}
	out_Color = tex * 1.5;*/

	/*vec4 tex = 0.120985 * texture2D( texture, ex_Texcoord + vec2( -2.0 / 768.0, 0.0 ) ) +
				0.176033 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 768.0, 0.0 ) ) +
				0.199471 * texture2D( texture, ex_Texcoord + vec2( 0.0 / 768.0, 0.0 ) ) +
				0.176033 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 768.0, 0.0 ) ) +
				0.120985 * texture2D( texture, ex_Texcoord + vec2( 2.0 / 768.0, 0.0 ) );*/
				
	/*vec4 tex = 0.0604927 * texture2D( texture, ex_Texcoord + vec2( -4.0 / 384.0, 0.0 ) ) +
				0.0752844 * texture2D( texture, ex_Texcoord + vec2( -3.0 / 384.0, 0.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( -2.0 / 384.0, 0.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 384.0, 0.0 ) ) +
				0.0997356 * texture2D( texture, ex_Texcoord + vec2( 0.0 / 384.0, 0.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 384.0, 0.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( 2.0 / 384.0, 0.0 ) ) +
				0.0752844 * texture2D( texture, ex_Texcoord + vec2( 3.0 / 384.0, 0.0 ) ) +
				0.0604927 * texture2D( texture, ex_Texcoord + vec2( 4.0 / 384.0, 0.0 ) );*/
				
	vec4 tex = 0.0752844 * texture2D( texture, ex_Texcoord + vec2( -3.0 / 384.0, 0.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( -2.0 / 384.0, 0.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( -1.0 / 384.0, 0.0 ) ) +
				0.0997356 * texture2D( texture, ex_Texcoord + vec2( 0.0 / 384.0, 0.0 ) ) +
				0.0966670 * texture2D( texture, ex_Texcoord + vec2( 1.0 / 384.0, 0.0 ) ) +
				0.0880163 * texture2D( texture, ex_Texcoord + vec2( 2.0 / 384.0, 0.0 ) ) +
				0.0752844 * texture2D( texture, ex_Texcoord + vec2( 3.0 / 384.0, 0.0 ) );
				
	out_Color = tex * 2.0;
	//out_Color = vec4( tex.r * 1.5, tex.g * 1.5, 0.0, 1.0 );
}
