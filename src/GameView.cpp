#include "GameView.h"

bool ProjCmp( const Projectile * proj1, const Projectile * proj2 ) {
	return std::string( typeid( *proj1 ).name( ) ) < std::string( typeid( *proj2 ).name( ) );
}

mat4f KeyframeMatrix( const JointAnimation::JointPose & pose ) {
	mat4f result = MultiplicitiveIdentity< mat4f >::Identity( );
	Sub< 3, 3 >::Matrix( result ) = RotationMatrixX( pose.rotation[ 0 ] ) * RotationMatrixY( pose.rotation[ 1 ] ) * RotationMatrixZ( pose.rotation[ 2 ] );
	Sub< 3, 1 >::Matrix( result, 0, 3 ) = Vec< 3 >::Tor( pose.position );
	return result;
}

GameView::GameView( ) : game( 0 ), cameraheight( 1024.0f ), doGlow( true ), doDistortion( true ), doDynamicLighting( true ), drawProjectiles( true ), drawParticles( true ), ambientIntensity( 1.0f / 32.0f ), walkAnimationSpeed( 0.012f ), bWriteShadowMaps( false ), shadowfboSize( 2048 ) { }

void GameView::init( ) {
	initFBO( );
	initShaders( );
	initTextures( );
	initHUD( );
	initPlayerModel( );
	initProjectileModels( );
	initItemModels( );
	//initActorText( );
}

void GameView::initFBO( ) {
	// deferred rendering
	glGenFramebuffers( 1, &gbufferfbo );
	glBindFramebuffer( GL_FRAMEBUFFER, gbufferfbo );

	glGenTextures( 1, &galbedoRT );
	glBindTexture( GL_TEXTURE_2D, galbedoRT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 768, 768, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

	glGenTextures( 1, &gnormalRT );
	glBindTexture( GL_TEXTURE_2D, gnormalRT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 768, 768, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

	glGenTextures( 1, &gdepthRT );
	glBindTexture( GL_TEXTURE_2D, gdepthRT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 768, 768, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, galbedoRT, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gnormalRT, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gdepthRT, 0 );

	// shadow fbo
	glGenFramebuffers( 1, &shadowfbo );
	glBindFramebuffer( GL_FRAMEBUFFER, shadowfbo );

	glGenTextures( 1, &shadowDepthRT );
	glBindTexture( GL_TEXTURE_2D, shadowDepthRT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowfboSize, shadowfboSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );

	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthRT, 0 );

}

void GameView::initHUD( ) {
	{
		float verts [] = { 
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f
		};

		float texcoords [] = {
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
		};

		hudquad.initVAO( );
		hudquad.addBuffers( 2 );
		hudquad.setBuffer( 0, verts, 6, 3, GL_STATIC_DRAW );
		hudquad.setBuffer( 1, texcoords, 6, 2, GL_STATIC_DRAW );
	}

	{
		float * verts = new float[ tesssize * tesssize * 3 ];
		for( int y = 0; y < tesssize; y++ ) {
			for( int x = 0; x < tesssize; x++ ) {
				verts[ y * tesssize * 3 + x * 3 + 0 ] = x / ( tesssize - 1.0f );
				verts[ y * tesssize * 3 + x * 3 + 1 ] = y / ( tesssize - 1.0f );
				verts[ y * tesssize * 3 + x * 3 + 2 ] = 0.0f;
			}
		}

		float * texcoords = new float[ tesssize * tesssize * 2 ];
		for( int y = 0; y < tesssize; y++ ) {
			for( int x = 0; x < tesssize; x++ ) {
				texcoords[ y * tesssize * 2 + x * 2 + 0 ] = x / ( tesssize - 1.0f );
				texcoords[ y * tesssize * 2 + x * 2 + 1 ] = y / ( tesssize - 1.0f );
			}
		}

		unsigned int * indices = new unsigned int[ ( tesssize - 1 ) * ( tesssize - 1 ) * 6 ];
		for( int y = 0; y < tesssize - 1; y++ ) {
			for( int x = 0; x < tesssize - 1; x++ ) {
				int topleft = y * tesssize + x;
				int topright = y * tesssize + x + 1;
				int bottomleft = ( y + 1 ) * tesssize + x;
				int bottomright = ( y + 1 ) * tesssize + x + 1;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 0 ] = topleft;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 1 ] = topright;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 2 ] = bottomright;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 3 ] = bottomright;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 4 ] = bottomleft;
				indices[ y * ( tesssize - 1 ) * 6 + x * 6 + 5 ] = topleft;
			}
		}

		tessquad.initVAO( );
		tessquad.addBuffers( 3 );
		tessquad.setBuffer( 0, verts, tesssize * tesssize, 3, GL_DYNAMIC_DRAW );
		tessquad.setBuffer( 1, texcoords, tesssize * tesssize, 2, GL_STATIC_DRAW );
		tessquad.setBuffer( 2, indices, ( tesssize - 1 ) * ( tesssize - 1 ) * 6, GL_STATIC_DRAW );

		delete [] verts;
		delete [] texcoords;
		delete [] indices;
	}
}

void GameView::initShaders( ) {
	// deferred rendering
	const std::string gthreedAttribs[] = { "in_Position", "in_Normal", "in_Texcoord" };
	gthreedShader.initShader( "./shaders/deferred/threed.vert", "./shaders/deferred/threed.frag", gthreedAttribs, gthreedAttribs + 3 );
	gthreedShader.getUniform( "model" );
	gthreedShader.getUniform( "view" );
	gthreedShader.getUniform( "projection" );
	gthreedShader.getUniform( "albedo" );

	const std::string gthreedanimAttribs[] = { "in_Position", "in_Normal", "in_Texcoord", "in_Boneid" };
	gthreedanimShader.initShader( "./shaders/deferred/threedanim.vert", "./shaders/deferred/threedanim.frag", gthreedanimAttribs, gthreedanimAttribs + 4 );
	gthreedanimShader.getUniform( "model" );
	gthreedanimShader.getUniform( "view" );
	gthreedanimShader.getUniform( "projection" );
	gthreedanimShader.getUniform( "bonematrices" );
	gthreedanimShader.getUniform( "albedo" );

	const std::string gthreednormmapAttribs[] = { "in_Position", "in_Normal", "in_Texcoord", "in_Tangent", "in_Bitangent" };
	gthreednormmapShader.initShader( "./shaders/deferred/threednormmap.vert", "./shaders/deferred/threednormmap.frag", gthreednormmapAttribs, gthreednormmapAttribs + 5 );
	gthreednormmapShader.getUniform( "model" );
	gthreednormmapShader.getUniform( "view" );
	gthreednormmapShader.getUniform( "projection" );
	gthreednormmapShader.getUniform( "albedo" );
	gthreednormmapShader.getUniform( "normalmap" );

	const std::string gpointlightAttribs[] = { "in_Position" };
	gpointlightShader.initShader( "./shaders/deferred/pointlight.vert", "./shaders/deferred/pointlight.frag", gpointlightAttribs, gpointlightAttribs + 1 );
	gpointlightShader.getUniform( "model" );
	gpointlightShader.getUniform( "view" );
	gpointlightShader.getUniform( "projection" );
	gpointlightShader.getUniform( "albedotex" );
	gpointlightShader.getUniform( "normaltex" );
	gpointlightShader.getUniform( "depthtex" );
	gpointlightShader.getUniform( "viewprojInv" );
	gpointlightShader.getUniform( "lightpos" );
	gpointlightShader.getUniform( "lightcolor" );

	const std::string gspotlightAttribs[] = { "in_Position" };
	gspotlightShader.initShader( "./shaders/deferred/spotlight.vert", "./shaders/deferred/spotlight.frag", gspotlightAttribs, gspotlightAttribs + 1 );
	gspotlightShader.getUniform( "model" );
	gspotlightShader.getUniform( "view" );
	gspotlightShader.getUniform( "projection" );
	gspotlightShader.getUniform( "albedotex" );
	gspotlightShader.getUniform( "normaltex" );
	gspotlightShader.getUniform( "depthtex" );
	gspotlightShader.getUniform( "viewprojInv" );
	gspotlightShader.getUniform( "lightpos" );
	gspotlightShader.getUniform( "lightcolor" );
	gspotlightShader.getUniform( "lightmatrix" );
	gspotlightShader.getUniform( "shadowmap" );

	const std::string gdirlightAttribs[] = { "in_Position" };
	gdirlightShader.initShader( "./shaders/deferred/dirlight.vert", "./shaders/deferred/dirlight.frag", gdirlightAttribs, gdirlightAttribs + 1 );
	gdirlightShader.getUniform( "model" );
	gdirlightShader.getUniform( "view" );
	gdirlightShader.getUniform( "projection" );
	gdirlightShader.getUniform( "albedotex" );
	gdirlightShader.getUniform( "normaltex" );
	gdirlightShader.getUniform( "depthtex" );
	gdirlightShader.getUniform( "viewprojInv" );
	gdirlightShader.getUniform( "lightdir" );
	gdirlightShader.getUniform( "lightcolor" );
	gdirlightShader.getUniform( "lightmatrix" );
	gdirlightShader.getUniform( "shadowmap" );

	const std::string gambientAttribs[] = { "in_Position" };
	gambientShader.initShader( "./shaders/deferred/ambient.vert", "./shaders/deferred/ambient.frag", gambientAttribs, gambientAttribs + 1 );
	gambientShader.getUniform( "model" );
	gambientShader.getUniform( "view" );
	gambientShader.getUniform( "projection" );
	gambientShader.getUniform( "albedotex" );
	gambientShader.getUniform( "normaltex" );
	gambientShader.getUniform( "depthtex" );

	const std::string plainAttribs[] = { "in_Position" };
	plainShader.initShader( "./shaders/plainshader.vert", "./shaders/plainshader.frag", plainAttribs, plainAttribs + 1 );
	plainShader.getUniform( "model" );
	plainShader.getUniform( "view" );
	plainShader.getUniform( "projection" );

	const std::string plainanimAttribs[] = { "in_Position", "in_Normal", "in_Texcoord", "in_Boneid" };
	plainanimShader.initShader( "./shaders/plainshaderanim.vert", "./shaders/plainshader.frag", plainanimAttribs, plainanimAttribs + 4 );
	plainanimShader.getUniform( "model" );
	plainanimShader.getUniform( "view" );
	plainanimShader.getUniform( "projection" );
	plainanimShader.getUniform( "bonematrices" );

	const std::string projAttribs[] = { "in_Position" };
	projShader.initShader( "./shaders/projshader.vert", "./shaders/projshader.frag", projAttribs, projAttribs + 1 );
	projShader.getUniform( "model" );
	projShader.getUniform( "view" );
	projShader.getUniform( "projection" );
	projShader.getUniform( "color" );

	const std::string threedtexAttribs[] = { "in_Position", "in_Texcoord" };
	threedtexShader.initShader( "./shaders/threedtex.vert", "./shaders/threedtex.frag", threedtexAttribs, threedtexAttribs + 2 );
	threedtexShader.getUniform( "model" );
	threedtexShader.getUniform( "view" );
	threedtexShader.getUniform( "projection" );
	threedtexShader.getUniform( "texture" );
	threedtexShader.getUniform( "texscale" );
}

void GameView::genTexture( std::istream & stream, GLuint & texid ) {
	glGenTextures( 1, &texid );
	glBindTexture( GL_TEXTURE_2D, texid );

	int texwidth, texheight, texdepth;
	unsigned char * texdata = ReadTGA( stream, texwidth, texheight, texdepth );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texwidth, texheight, 0, texdepth == 24 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, texdata );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	delete [] texdata;
}

void GameView::genNormalTexture( std::istream & stream, GLuint & texid ) {
	glGenTextures( 1, &texid );
	glBindTexture( GL_TEXTURE_2D, texid );

	int texwidth, texheight, texdepth;
	unsigned char * texdata = ReadTGA( stream, texwidth, texheight, texdepth );
	unsigned char * normdata = new unsigned char[ texwidth * texheight * 3 ];
	
	for( int y = 0; y < texheight; y++ ) {
		for( int x = 0; x < texwidth; x++ ) {
			unsigned char p = texdata[ y * texwidth * texdepth / 8 + x * texdepth / 8 + 0 ];
			unsigned char pright = texdata[ y * texwidth * texdepth / 8 + ( ( x + 1 ) % texwidth ) * texdepth / 8 + 0 ];
			unsigned char pdown = texdata[ ( ( y + 1 ) % texheight ) * texwidth * texdepth / 8 + x * texdepth / 8 + 0 ];
			
			vec3f vec1 = Vector3( 1.0f, 0.0f, ( (float)pright - (float)p ) / 255.0f );
			vec3f vec2 = Vector3( 0.0f, -1.0f, ( (float)pdown - (float)p ) / 255.0f );
			vec3f normal = Normalize( CrossProduct( vec2, vec1 ) );
			
			normdata[ y * texwidth * 3 + x * 3 + 0 ] = (unsigned char)( 255.0f * ( normal[ 0 ] * 0.5f + 0.5f ) );
			normdata[ y * texwidth * 3 + x * 3 + 1 ] = (unsigned char)( 255.0f * ( normal[ 1 ] * 0.5f + 0.5f ) );
			normdata[ y * texwidth * 3 + x * 3 + 2 ] = (unsigned char)( 255.0f * ( normal[ 2 ] * 0.5f + 0.5f ) );
		}
	}

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, normdata );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	delete [] normdata;
	delete [] texdata;
}

void GameView::initBrushTextures( ) {
	// load textures from brushes
	for( std::map< std::string, DrawableObject >::iterator iter = brushPolygons.begin( ); iter != brushPolygons.end( ); ++iter ) {
		const std::string & texname = iter->first;

		std::ifstream tgastream( ( "./textures/" + texname ).c_str( ), std::ios::binary );
		if( tgastream ) {
			GLuint textureID;
			genTexture( tgastream, textureID );
			textures[ texname ] = textureID;
			
			std::ifstream hmapstream( ( "./textures/" + texname.substr( 0, texname.rfind( "." ) ) + "height.tga" ).c_str( ), std::ios::binary );
			if( hmapstream ) {
				GLuint hmapID;
				genNormalTexture( hmapstream, hmapID );
				normtextures[ texname ] = hmapID;
			}
		}
	}
}

void GameView::initTextures( ) {
	// create default texture for missing texture files
	{
		glGenTextures( 1, &notexTex );
		glBindTexture( GL_TEXTURE_2D, notexTex );

		int texwidth = 256, texheight = 256;
		unsigned char * texdata = new unsigned char[ texwidth * texheight * 3 ];
		for( int y = 0; y < texheight; y++ ) {
			for( int x = 0; x < texwidth; x++ ) {
				// fill texture with sinusoidal functions
				texdata[ y * texwidth * 3 + x * 3 + 0 ] = (unsigned char)( cos( x * 0.1f ) * 127.0f + 127.0f );
				texdata[ y * texwidth * 3 + x * 3 + 1 ] = (unsigned char)( sin( y * 0.1f ) * 127.0f + 127.0f );
				texdata[ y * texwidth * 3 + x * 3 + 2 ] = (unsigned char)( cos( x * y * 0.1f ) * 127.0f + 127.0f );
			}
		}

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		delete [] texdata;
	}

	// load hud textures
	{
		std::ifstream stream( "./textures/plus.tga", std::ios::binary );
		genTexture( stream, plusTex );
	}

	{
		std::ifstream stream( "./textures/powerup.tga", std::ios::binary );
		genTexture( stream, powerupTex );
	}

	{
		std::ifstream stream( "./textures/homing.tga", std::ios::binary );
		genTexture( stream, homingTex );
	}

	{
		std::ifstream stream( "./textures/attackspeed.tga", std::ios::binary );
		genTexture( stream, attackspeedTex );
	}

	{
		std::ifstream stream( "./textures/speed.tga", std::ios::binary );
		genTexture( stream, speedTex );
	}

	{
		std::ifstream stream( "./textures/knife.tga", std::ios::binary );
		genTexture( stream, knifeTex );
		projTex[ typeid( ProjectileMelee ).name( ) ] = knifeTex;
	}

	{
		std::ifstream stream( "./textures/machinegun.tga", std::ios::binary );
		genTexture( stream, machinegunTex );
		projTex[ typeid( ProjectileBullet ).name( ) ] = machinegunTex;
	}

	{
		std::ifstream stream( "./textures/shotgun.tga", std::ios::binary );
		genTexture( stream, shotgunTex );
		projTex[ typeid( ProjectileBounceLaser ).name( ) ] = shotgunTex;
	}

	{
		std::ifstream stream( "./textures/rocket.tga", std::ios::binary );
		genTexture( stream, rocketTex );
		projTex[ typeid( ProjectileRocket ).name( ) ] = rocketTex;
	}

	{
		std::ifstream stream( "./textures/grenade.tga", std::ios::binary );
		genTexture( stream, grenadeTex );
		projTex[ typeid( ProjectileGrenade ).name( ) ] = grenadeTex;
	}

	{
		std::ifstream stream( "./textures/flame.tga", std::ios::binary );
		genTexture( stream, flameTex );
		projTex[ typeid( ProjectileFlame ).name( ) ] = flameTex;
	}

	{
		std::ifstream stream( "./textures/bfg.tga", std::ios::binary );
		genTexture( stream, bfgTex );
		projTex[ typeid( ProjectileBFG ).name( ) ] = bfgTex;
	}

	{
		std::ifstream stream( "./textures/letters.tga", std::ios::binary );
		genTexture( stream, lettersTex );
	}
}

void GameView::initPlayerModel( ) {
	{
		std::ifstream meshstream( "./meshes/zombie02.ms3d", std::ios::binary );
		MS3DToMesh meshloader;
		MS3D::Load( meshstream, meshloader );

		boneMatrices = meshloader.GetBoneMatrixContainer( );
		boneMatrices.Calculate( );

		std::vector< JointAnimation > jointAnimations = meshloader.GetJointAnimations( );
		/*for( size_t i = 0; i < jointAnimations.size( ); i++ ) {
			walkAnimation.push_back( jointAnimations[ i ].Extract( 2.0f / meshloader.GetAnimationFPS( ), 20.0f / meshloader.GetAnimationFPS( ) ) );
		}*/

		{
			std::ifstream infostream( "zombie02.txt", std::ios::binary );
			std::map< std::string, std::pair< int, int > > animationinfo;
			while( infostream ) {
				std::string animationname;
				int startframe, endframe;
				infostream >> animationname >> startframe >> endframe;
				if( infostream ) {
					animationinfo[ animationname ] = std::make_pair( startframe, endframe );
					std::cout << "animation: " << animationname << std::endl;
				}
			}
			infostream.close( );

			for( std::map< std::string, std::pair< int, int > >::iterator iter = animationinfo.begin( ); iter != animationinfo.end( ); ++iter ) {
				animationMap[ iter->first ].resize( jointAnimations.size( ) );
				for( size_t i = 0; i < jointAnimations.size( ); i++ ) {
					animationMap[ iter->first ][ i ] = jointAnimations[ i ].Extract( iter->second.first / meshloader.GetAnimationFPS( ), iter->second.second / meshloader.GetAnimationFPS( ) );
				}
			}
		}

		std::vector< float > vertices = meshloader.GetVertices( );
		std::vector< float > normals = meshloader.GetNormals( );

		InverseTransform( vertices, normals, meshloader.GetBoneIds( ), boneMatrices.GetMatrices( ) );

		playerModel.initVAO( );
		playerModel.addBuffers( 5 );
		playerModel.setBuffer( 0, &vertices[ 0 ], meshloader.GetVertices( ).size( ) / 3, 3, GL_STATIC_DRAW );
		playerModel.setBuffer( 1, &normals[ 0 ], meshloader.GetVertices( ).size( ) / 3, 3, GL_STATIC_DRAW );
		playerModel.setBuffer( 2, &meshloader.GetTexcoords( )[ 0 ], meshloader.GetVertices( ).size( ) / 3, 2, GL_STATIC_DRAW );
		playerModel.setBuffer( 3, &meshloader.GetBoneIds( )[ 0 ], meshloader.GetVertices( ).size( ) / 3, 1, GL_STATIC_DRAW );
		playerModel.setBuffer( 4, &meshloader.GetGroups( ).front( ).second[ 0 ], meshloader.GetGroups( ).front( ).second.size( ), GL_STATIC_DRAW );

		playerModelSize = 4.0f;
	}

	{
		std::ifstream texstream( "./meshes/zombie.tga", std::ios::binary );
		genTexture( texstream, actorTex );
	}
}

void GameView::initProjectileModels( ) {
	{
		float * verts;
		int nverts;

		genCube( verts, nverts, 1.0f, 1.0f, 1.0f );

		cubeShape.initVAO( );
		cubeShape.addBuffers( 1 );
		cubeShape.setBuffer( 0, verts, nverts, 3, GL_STATIC_DRAW );

		delete [] verts;
	}
	{
		float * verts;
		int nverts;

		genCylinder( verts, nverts, 0.0f, 1.0f, 1.0f, 8, 8 );

		coneShape.initVAO( );
		coneShape.addBuffers( 1 );
		coneShape.setBuffer( 0, verts, nverts, 3, GL_STATIC_DRAW );

		delete [] verts;
	}
	{
		float * verts;
		int nverts;

		genSphere( verts, nverts, 1.0f, 8, 8 );

		sphereShape.initVAO( );
		sphereShape.addBuffers( 1 );
		sphereShape.setBuffer( 0, verts, nverts, 3, GL_STATIC_DRAW );

		delete [] verts;
	}

	projDrawFuncs[ typeid( ProjectileMelee ).name( ) ] = &GameView::drawProjectileMelee;
	projDrawFuncs[ typeid( ProjectileBullet ).name( ) ] = &GameView::drawProjectileBullet;
	projDrawFuncs[ typeid( ProjectileBounceLaser ).name( ) ] = &GameView::drawProjectileBounceLaser;
	projDrawFuncs[ typeid( ProjectileRocket ).name( ) ] = &GameView::drawProjectileRocket;
	projDrawFuncs[ typeid( ProjectileGrenade ).name( ) ] = &GameView::drawProjectileGrenade;
	projDrawFuncs[ typeid( ProjectileFlame ).name( ) ] = &GameView::drawProjectileFlame;
	projDrawFuncs[ typeid( ProjectileBFG ).name( ) ] = &GameView::drawProjectileBFG;
}

void GameView::initItemModels( ) {
	{
		float verts [] = {
			1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,

			-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,

			-1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,

			1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,

			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

			1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f
		};

		float texcoords [] = {
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f
		};

		weaponBox.initVAO( );
		weaponBox.addBuffers( 2 );
		weaponBox.setBuffer( 0, verts, 12 * 3, 3, GL_STATIC_DRAW );
		weaponBox.setBuffer( 1, texcoords, 12 * 3, 2, GL_STATIC_DRAW );
	}

	itemDrawFuncs[ typeid( ItemWeapon ).name( ) ] = &GameView::drawItemWeapon;
	itemDrawFuncs[ typeid( ItemPowerup ).name( ) ] = &GameView::drawItemPowerup;
}

void GameView::uninit( ) {
	weaponBox.destroyVAO( );
	sphereShape.destroyVAO( );
	coneShape.destroyVAO( );
	cubeShape.destroyVAO( );
	playerModel.destroyVAO( );
	hudquad.destroyVAO( );
	tessquad.destroyVAO( );

	uninitTextures( );
	uninitBrushes( );
	uninitActorText( );

	// deferred rendering
	gdirlightShader.destroyShader( );
	gspotlightShader.destroyShader( );
	gpointlightShader.destroyShader( );
	gthreednormmapShader.destroyShader( );
	gthreedanimShader.destroyShader( );
	gthreedShader.destroyShader( );
	plainShader.destroyShader( );
	plainanimShader.destroyShader( );
	threedtexShader.destroyShader( );
	projShader.destroyShader( );

	glDeleteTextures( 1, &galbedoRT );
	glDeleteTextures( 1, &gnormalRT );
	glDeleteTextures( 1, &gdepthRT );
	glDeleteFramebuffers( 1, &gbufferfbo );

	glDeleteTextures( 1, &shadowDepthRT );
	glDeleteFramebuffers( 1, &shadowfbo );
}

void GameView::uninitActorText( ) {
	for( std::map< std::string, DrawableObject >::iterator iter = actorNameText.begin( ); iter != actorNameText.end( ); ++iter ) {
		iter->second.destroyVAO( );
	}
	actorNameText.clear( );

	for( std::map< std::string, DrawableObject >::iterator iter = actorScoreText.begin( ); iter != actorScoreText.end( ); ++iter ) {
		iter->second.destroyVAO( );
	}
	actorScoreText.clear( );

	actorKills.clear( );
	actorDeaths.clear( );
	actorDamageDealt.clear( );
	actorDamageReceived.clear( );

	/// ehhhh this shouldn't be here
	walkTimer.clear( );
	actorAnimation.clear( );
}

void GameView::uninitTextures( ) {
	glDeleteTextures( 1, &bfgTex );
	glDeleteTextures( 1, &flameTex );
	glDeleteTextures( 1, &grenadeTex );
	glDeleteTextures( 1, &rocketTex );
	glDeleteTextures( 1, &shotgunTex );
	glDeleteTextures( 1, &machinegunTex );
	glDeleteTextures( 1, &shotgunTex );
	glDeleteTextures( 1, &plusTex );
	glDeleteTextures( 1, &lettersTex );
	projTex.clear( );
}

void GameView::clearObjects( ) {
	particles.clear( );
	bloodparticles.clear( );
	explosions.clear( );
	itemparticles.clear( );
	pointlights.clear( );
	spotlights.clear( );
	directionallights.clear( );
}

void GameView::uninitBrushes( ) {
	for( std::map< std::string, DrawableObject >::iterator iter = brushPolygons.begin( ); iter != brushPolygons.end( ); ++iter ) {
		DrawableObject & obj = iter->second;
		obj.destroyVAO( );
	}

	brushPolygons.clear( );

	for( std::map< std::string, GLuint >::iterator iter = textures.begin( ); iter != textures.end( ); ++iter ) {
		GLuint textureID = iter->second;
		glDeleteTextures( 1, &textureID );
	}

	textures.clear( );
}

void GameView::setGame( const GameModel * _game ) {
	game = _game;
}

void GameView::setCamera( const ActorModel * _camera ) {
	camera = _camera;
}

void GameView::setFirstPerson( ) {
	firstperson = true;
}

void GameView::setThirdPerson( ) {
	firstperson = false;
}

void GameView::setGlow( bool _doGlow ) {
	doGlow = _doGlow;
}

void GameView::setDistortion( bool _doDistortion ) {
	doDistortion = _doDistortion;
}

void GameView::setDynamicLighting( bool _doDynamicLighting ) {
	doDynamicLighting = _doDynamicLighting;
}

void GameView::setDrawProjectiles( bool _drawProjectiles ) {
	drawProjectiles = _drawProjectiles;
}

void GameView::setDrawParticles( bool _drawParticles ) {
	drawParticles = _drawParticles;
}

void GameView::setAmbientIntensity( float _ambientIntensity ) {
	ambientIntensity = _ambientIntensity;
}

void GameView::setWalkAnimationSpeed( float _walkAnimationSpeed ) {
	walkAnimationSpeed = _walkAnimationSpeed;
}

void GameView::writeShadowMaps( const std::string & _shadowfilename ) {
	bWriteShadowMaps = true;
	shadowfilename = _shadowfilename;
}

void GameView::getProjectionMatrix( float * projection ) const {
	perspective( projection, 3.1415926f / 3.0f, 1.0f, 1.0f, 4096.0f );
}

void GameView::getViewMatrix( float * view ) const {
	if( camera ) {
		const vec3f & viewpos = camera->getPosition( );
		const vec3f & viewdir = camera->getOrientation( );
		if( firstperson ) {
			//lookat( view, viewpos[ 0 ], viewpos[ 1 ] + 48.0f, viewpos[ 2 ], viewpos[ 0 ] + viewdir[ 0 ], viewpos[ 1 ] + viewdir[ 1 ] + 48.0f, viewpos[ 2 ] + viewdir[ 2 ], 0.0f, 1.0f, 0.0f );
			lookat( view, viewpos[ 0 ] - viewdir[ 0 ] * 128.0f, 128.0f, viewpos[ 2 ] - viewdir[ 2 ] * 128.0f, viewpos[ 0 ], 96.0f, viewpos[ 2 ], 0.0f, 1.0f, 0.0f );
		} else {
			lookat( view, viewpos[ 0 ], cameraheight, viewpos[ 2 ], viewpos[ 0 ], 0.0f, viewpos[ 2 ], 0.0f, 0.0f, -1.0f );
		}
	} else {
		lookat( view, 0.0f, cameraheight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f );
	}
}

vec3f GameView::getViewPos( ) const {
	if( camera ) {
		const vec3f & viewpos = camera->getPosition( );
		const vec3f & viewdir = camera->getOrientation( );
		if( firstperson ) {
			return Vector3( viewpos[ 0 ] - viewdir[ 0 ] * 128.0f, 128.0f, viewpos[ 2 ] - viewdir[ 2 ] * 128.0f );
		} else {
			return Vector3( viewpos[ 0 ], cameraheight, viewpos[ 2 ] );
		}
	} else {
		return Vector3( 0.0f, cameraheight, 0.0f );
	}
}

vec3f GameView::getViewDir( ) const {
	if( camera ) {
		const vec3f & viewpos = camera->getPosition( );
		const vec3f & viewdir = camera->getOrientation( );
		if( firstperson ) {			//std::vector< mat4f > transformations( walkAnimation.size( ) );

			return Vector3N( viewdir[ 0 ] * 128.0f, 96.0f - 128.0f, viewdir[ 2 ] * 128.0f );
			//lookat( view, viewpos[ 0 ] - viewdir[ 0 ] * 128.0f, 128.0f, viewpos[ 2 ] - viewdir[ 2 ] * 128.0f, viewpos[ 0 ], 96.0f, viewpos[ 2 ], 0.0f, 1.0f, 0.0f );
		} else {
			//lookat( view, viewpos[ 0 ], cameraheight, viewpos[ 2 ], viewpos[ 0 ], 0.0f, viewpos[ 2 ], 0.0f, 0.0f, -1.0f );
			return Vector3( 0.0f, -1.0f, 0.0f );
		}
	} else {
		//lookat( view, 0.0f, cameraheight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f );
		return Vector3( 0.0f, -1.0f, 0.0f );
	}
}

void GameView::updateActorText( const ActorModel & actor ) {
	if( actorKills.find( actor.getName( ) ) != actorKills.end( ) ) {
		int kills = actorKills[ actor.getName( ) ];
		int deaths = actorDeaths[ actor.getName( ) ];
		float damagedealt = actorDamageDealt[ actor.getName( ) ];
		float damagereceived = actorDamageReceived[ actor.getName( ) ];
		float kdratio = (float)kills / deaths;
		float damageratio = damagedealt / damagereceived;
		std::ostringstream scorestream;
		scorestream.precision( 2 );
		scorestream << std::fixed;
		scorestream << "[" << kills << "/" << deaths << "]=" << kdratio << "   " 
			<< "[" << (int)damagedealt << "/" << (int)damagereceived << "]=" << damageratio;

		float * verts = createTextVerts( scorestream.str( ) ), * texcoords = createTextTexcoords( scorestream.str( ) );
		unsigned int * indices = createTextIndices( scorestream.str( ) );
		actorScoreText[ actor.getName( ) ].setBuffer( 0, verts, scorestream.str( ).size( ) * 4, 3, GL_DYNAMIC_DRAW );
		actorScoreText[ actor.getName( ) ].setBuffer( 1, texcoords, scorestream.str( ).size( ) * 4, 2, GL_DYNAMIC_DRAW );
		actorScoreText[ actor.getName( ) ].setBuffer( 2, indices, scorestream.str( ).size( ) * 6, GL_DYNAMIC_DRAW );
		delete [] verts;
		delete [] texcoords;
		delete [] indices;
	}
}

void GameView::itemPickedUp( const ActorModel & actor, const Item * item, float starttime ) {
	itemparticles.push_back( Particle( item->getPosition( ), vec3f( 0.0f ), starttime, 0.5f ) );
}

void GameView::addActor( const ActorModel & actor ) {
	const std::string & name = actor.getName( );

	walkTimer[ name ] = 0.0f;
	actorAnimation[ name ] = "walk1";

	actorKills[ name ] = 0;
	actorDeaths[ name ] = 0;
	actorDamageDealt[ name ] = 0.0f;
	actorDamageReceived[ name ] = 0.0f;

	{
		actorNameText[ name ].initVAO( );
		actorNameText[ name ].addBuffers( 3 );
		float * verts = createTextVerts( name ), * texcoords = createTextTexcoords( name );
		unsigned int * indices = createTextIndices( name );
		actorNameText[ name ].setBuffer( 0, verts, name.size( ) * 4, 3, GL_STATIC_DRAW );
		actorNameText[ name ].setBuffer( 1, texcoords, name.size( ) * 4, 2, GL_STATIC_DRAW );
		actorNameText[ name ].setBuffer( 2, indices, name.size( ) * 6, GL_STATIC_DRAW );
		delete [] verts;
		delete [] texcoords;
		delete [] indices;
	}

	{
		actorKills[ name ] = 0;
		actorDeaths[ name ] = 0;
		actorDamageDealt[ name ] = 0.0f;
		actorDamageReceived[ name ] = 0.0f;

		actorScoreText[ name ].initVAO( );
		actorScoreText[ name ].addBuffers( 3 );
		float * verts = createTextVerts( name ), * texcoords = createTextTexcoords( name );
		unsigned int * indices = createTextIndices( name );
		actorScoreText[ name ].setBuffer( 0, createTextVerts( "0/0,0/0" ), 7 * 4, 3, GL_DYNAMIC_DRAW );
		actorScoreText[ name ].setBuffer( 1, createTextTexcoords( "0/0,0/0" ), 7 * 4, 2, GL_DYNAMIC_DRAW );
		actorScoreText[ name ].setBuffer( 2, createTextIndices( "0/0,0/0" ), 7 * 6, GL_DYNAMIC_DRAW );
		delete [] verts;
		delete [] texcoords;
		delete [] indices;
	}
}

void GameView::removeActor( const ActorModel & actor ) {
	if( camera == &actor ) {
		camera = 0;
	}

	walkTimer.erase( actor.getName( ) );
	actorAnimation.erase( actor.getName( ) );

	actorNameText[ actor.getName( ) ].destroyVAO( );
	actorNameText.erase( actor.getName( ) );

	actorScoreText[ actor.getName( ) ].destroyVAO( );
	actorScoreText.erase( actor.getName( ) );

	actorKills.erase( actor.getName( ) );
	actorDeaths.erase( actor.getName( ) );
	actorDamageDealt.erase( actor.getName( ) );
	actorDamageReceived.erase( actor.getName( ) );

	muzzleFlares.erase( &actor );
}

void GameView::actorSpawned( const ActorModel & actor, float starttime ) {
}

void GameView::actorDamaged( const ActorModel * attacker, const ActorModel & attackee, float damage, float starttime ) {
	if( attacker && actorDamageDealt.find( attacker->getName( ) ) != actorDamageDealt.end( ) ) {
		actorDamageDealt[ attacker->getName( ) ] += damage;
	}
	if( actorDamageReceived.find( attackee.getName( ) ) != actorDamageReceived.end( ) ) {
		actorDamageReceived[ attackee.getName( ) ] += damage;
	}

	if( attacker ) {
		updateActorText( *attacker );
	}
	updateActorText( attackee );

	for( int i = 0; i < 32; i++ ) {
		vec3f velocity = Vector3( (float)rand( ) / RAND_MAX - 0.5f, (float)rand( ) / RAND_MAX - 0.5f, (float)rand( ) / RAND_MAX - 0.5f );
		bloodparticles.push_back( Particle( attackee.getPosition( ), velocity * 8.0f, starttime, 0.75f ) );
	}
}

void GameView::actorKilled( const ActorModel * killer, const ActorModel & killee, float starttime ) {
	if( killer && actorKills.find( killer->getName( ) ) != actorKills.end( ) ) {
		actorKills[ killer->getName( ) ]++;
	}
	if( actorDeaths.find( killee.getName( ) ) != actorDeaths.end( ) ) {
		actorDeaths[ killee.getName( ) ]++;
	}

	if( killer ) {
		updateActorText( *killer );
	}
	updateActorText( killee );
}

void GameView::projectileCreation( const ActorModel & actor, const vec3f & position, float starttime ) {
	muzzleFlares[ &actor ] = starttime;
}

void GameView::projectileCollision( const vec3f & position, const vec3f & normal, float starttime ) {
	for( int i = 0; i < 16; i++ ) {
		vec3f velocity = Vector3( (float)rand( ) / RAND_MAX - 0.5f, (float)rand( ) / RAND_MAX - 0.5f, (float)rand( ) / RAND_MAX - 0.5f );
		float dp = DotProduct( velocity, normal );
		if( dp < 0.0f ) {
			velocity += -2.0f * dp * normal;
		}
		particles.push_back( Particle( position, velocity * 8.0f, starttime, 0.5f ) );
	}
}

void GameView::projectileExplosion( const vec3f & position, float starttime ) {
	explosions.push_back( Particle( position, vec3f( 0.0f ), starttime, 0.25f ) );
}

/*void GameView::addLight( const vec3f & position, const vec3f & color, float power ) {
	lights.push_back( Light( position, color, power ) );
}*/

void GameView::addPointLight( const PointLight & pointlight ) {
	pointlights.push_back( pointlight );
}

void GameView::addSpotLight( const SpotLight & spotlight ) {
	spotlights.push_back( spotlight );
}

void GameView::addDirectionalLight( const DirectionalLight & directionallight ) {
	directionallights.push_back( directionallight );
}

void GameView::setBrushes( std::map< std::string, std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > > & texpolygons ) {
	typedef std::map< std::string, std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > >::iterator texpolygonsiter;

	std::map< std::string, std::pair< int, int > > texdimensions;
	for( texpolygonsiter iter = texpolygons.begin( ); iter != texpolygons.end( ); ++iter ) {
		const std::string & texname = iter->first;

		int tgawidth, tgaheight, tgadepth;
		std::ifstream tgastream( texname.c_str( ), std::ios::binary );
		if( tgastream ) {
			ReadTGA( tgastream, tgawidth, tgaheight, tgadepth );
			texdimensions[ texname ] = std::make_pair( tgawidth, tgaheight );
		} else {
			texdimensions[ texname ] = std::make_pair( 256, 256 );
		}
	}

	for( texpolygonsiter iter = texpolygons.begin( ); iter != texpolygons.end( ); ++iter ) {
		const std::string & texname = iter->first;
		std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > & polygons = iter->second;

		std::vector< float > verts, texcoords, normals, tangents, bitangents;
		std::vector< unsigned int > indices;

		for( std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > >::iterator piter = polygons.begin( ); piter != polygons.end( ); piter++ ) {
			PlanarPolygon< 3, float, TexturedPlane< 3, float > > & polygon = *piter;
			const vec3f & normal = polygon.plane.GetNormal( );

			for( size_t i = 1; i < polygon.points.size( ) - 1; i++ ) {
				indices.push_back( verts.size( ) / 3 + 0 );
				indices.push_back( verts.size( ) / 3 + i + 1 );
				indices.push_back( verts.size( ) / 3 + i );
			}

			for( std::list< vec3f >::iterator viter = polygon.points.begin( ); viter != polygon.points.end( ); viter++ ) {
				vec3f & point = *viter;

				verts.push_back( point[ 0 ] );
				verts.push_back( point[ 1 ] );
				verts.push_back( point[ 2 ] );

				normals.push_back( normal[ 0 ] );
				normals.push_back( normal[ 1 ] );
				normals.push_back( normal[ 2 ] );

				if( abs( normal[ 0 ] ) > abs( normal[ 1 ] ) && abs( normal[ 0 ] ) > abs( normal[ 2 ] ) ) {
					texcoords.push_back( point[ 1 ] / texdimensions[ texname ].first );
					texcoords.push_back( point[ 2 ] / texdimensions[ texname ].second );
				} else if( abs( normal[ 1 ] ) > abs( normal[ 0 ] ) && abs( normal[ 1 ] ) > abs( normal[ 2 ] ) ) {
					texcoords.push_back( point[ 0 ] / texdimensions[ texname ].first );
					texcoords.push_back( point[ 2 ] / texdimensions[ texname ].second );
				} else if( abs( normal[ 2 ] ) > abs( normal[ 0 ] ) && abs( normal[ 2 ] ) > abs( normal[ 1 ] ) ) {
					texcoords.push_back( point[ 0 ] / texdimensions[ texname ].first );
					texcoords.push_back( point[ 1 ] / texdimensions[ texname ].second );
				}
			}

			for( std::list< vec3f >::iterator viter = polygon.points.begin( ); viter != polygon.points.end( ); viter++ ) {
				vec3f & point = *viter;
				
				vec3f p0 = Vec< 3 >::Tor( &verts[ verts.size( ) - polygon.points.size( ) * 3 ] );
				vec3f p1 = Vec< 3 >::Tor( &verts[ verts.size( ) - polygon.points.size( ) * 3 + 3 ] );
				vec3f p2 = Vec< 3 >::Tor( &verts[ verts.size( ) - polygon.points.size( ) * 3 + 6 ] );
				
				vec3f q1 = p1 - p0, q2 = p2 - p0;
				
				float u0 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 0 ];
				float v0 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 1 ];
				float u1 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 2 ];
				float v1 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 3 ];
				float u2 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 4 ];
				float v2 = texcoords[ texcoords.size( ) - polygon.points.size( ) * 2 + 5 ];
				
				float s1 = u1 - u0, t1 = v1 - v0;
				float s2 = u2 - u0, t2 = v2 - v0;
				
				float denom = s1 * t2 - s2 * t1;
				float tangent [] = { ( q1[ 0 ] * t2 - q2[ 0 ] * t1 ) / denom,
										( q1[ 1 ] * t2 - q2[ 1 ] * t1 ) / denom,
										( q1[ 2 ] * t2 - q2[ 2 ] * t1 ) / denom };
				Vec< 3 >::Tor( tangent ) = Normalize( Vec< 3 >::Tor( tangent ) );
				float bitangent [] = { ( -q1[ 0 ] * s2 + q2[ 0 ] * s1 ) / denom,
										( -q1[ 1 ] * s2 + q2[ 1 ] * s1 ) / denom,
										( -q1[ 2 ] * s2 + q2[ 2 ] * s1 ) / denom };
				Vec< 3 >::Tor( bitangent ) = Normalize( Vec< 3 >::Tor( bitangent ) );
										
				tangents.push_back( tangent[ 0 ] );
				tangents.push_back( tangent[ 1 ] );
				tangents.push_back( tangent[ 2 ] );
				
				bitangents.push_back( bitangent[ 0 ] );
				bitangents.push_back( bitangent[ 1 ] );
				bitangents.push_back( bitangent[ 2 ] );
			}
		}

		brushPolygons[ texname ] = DrawableObject( );
		brushPolygons[ texname ].initVAO( );
		brushPolygons[ texname ].addBuffers( 6 );
		brushPolygons[ texname ].setBuffer( 0, &verts[ 0 ], verts.size( ) / 3, 3, GL_STATIC_DRAW );
		brushPolygons[ texname ].setBuffer( 1, &normals[ 0 ], normals.size( ) / 3, 3, GL_STATIC_DRAW );
		brushPolygons[ texname ].setBuffer( 2, &texcoords[ 0 ], texcoords.size( ) / 2, 2, GL_STATIC_DRAW );
		brushPolygons[ texname ].setBuffer( 3, &tangents[ 0 ], tangents.size( ) / 3, 3, GL_STATIC_DRAW );
		brushPolygons[ texname ].setBuffer( 4, &bitangents[ 0 ], bitangents.size( ) / 3, 3, GL_STATIC_DRAW );
		brushPolygons[ texname ].setBuffer( 5, &indices[ 0 ], indices.size( ), GL_STATIC_DRAW );
	}
}

void GameView::update( float time ) {
	// update particles
	for( std::list< Particle >::iterator iter = particles.begin( ); iter != particles.end( ); ++iter ) {
		Particle & particle = *iter;

		particle.position += particle.velocity;
		particle.velocity += Vector3( 0.0f, -0.25f, 0.0f );
	}

	for( std::list< Particle >::iterator iter = bloodparticles.begin( ); iter != bloodparticles.end( ); ++iter ) {
		Particle & particle = *iter;

		particle.position += particle.velocity;
		particle.velocity += Vector3( 0.0f, -0.25f, 0.0f );
	}

	particles.remove_if( std::bind2nd( std::mem_fun_ref( &Particle::isDead ), time ) );
	bloodparticles.remove_if( std::bind2nd( std::mem_fun_ref( &Particle::isDead ), time ) );
	explosions.remove_if( std::bind2nd( std::mem_fun_ref( &Particle::isDead ), time ) );
	itemparticles.remove_if( std::bind2nd( std::mem_fun_ref( &Particle::isDead ), time ) );

	// update walk timer
	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;

		walkTimer[ actor.getName( ) ] += walkAnimationSpeed * Norm( actor.getVelocity( ) );
	}

	// update view params
	objdist = 0.0f;

	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;

		//if( actor.getState( ) == "idle" ) {
		if( actor.getHealth( ) > 0.0f ) {
			vec3f campos = camera ? camera->getPosition( ) : Vector3( 0.0f, 0.0f, 0.0f );
			if( abs( actor.getPosition( )[ 0 ] - campos[ 0 ] ) + actor.getBoundingRadius( ) * 8.0f > objdist ) {
				objdist = abs( actor.getPosition( )[ 0 ] - campos[ 0 ] ) + actor.getBoundingRadius( ) * 8.0f;
			}
			if( abs( actor.getPosition( )[ 2 ] - campos[ 2 ] ) + actor.getBoundingRadius( ) * 8.0f > objdist ) {
				objdist = abs( actor.getPosition( )[ 2 ] - campos[ 2 ] ) + actor.getBoundingRadius( ) * 8.0f;
			}
		}
	}

	if( objdist < 256.0f ) {
		objdist = 256.0f;
	}
	if( objdist < 256.0f ) {
		objdist = 256.0f;
	}
	if( objdist > 448.0f ) {
		objdist = 448.0f;
	}
	if( objdist > 448.0f ) {
		objdist = 448.0f;
	}

	float fov = 3.1415926f / 3.0f;
	float cheight = objdist / tan( fov / 2.0f );
	cameraheight += ( cheight - cameraheight ) / 8.0f;
}

void GameView::drawActors( Shader & shader, const ActorModel * actorIgnore ) {
	playerModel.bindBuffers( );
	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;

		if( actorIgnore != &actor && actor.getHealth( ) > 0.0f ) {
			vec3f upvec = Vector3( 0.0f, 1.0f, 0.0f );
			const vec3f & forwardvec = actor.getOrientation( );
			vec3f rightvec = CrossProduct( upvec, forwardvec );

			std::vector< JointAnimation > & animation = animationMap[ actorAnimation[ actor.getName( ) ] ];
			//std::vector< mat4f > transformations( walkAnimation.size( ) );
			std::vector< mat4f > transformations( animation.size( ) );
			//for( size_t i = 0; i < walkAnimation.size( ); i++ ) {
			for( size_t i = 0; i < animation.size( ); i++ ) {
				//transformations[ i ] = KeyframeMatrix( walkAnimation[ i ].GetInterpolatedKeyframe( walkTimer[ actor.getName( ) ], true ) );
				transformations[ i ] = KeyframeMatrix( animation[ i ].GetInterpolatedKeyframe( walkTimer[ actor.getName( ) ], true ) );
			}
			boneMatrices.Calculate( transformations.begin( ), transformations.end( ) );
			shader.setUniform( "bonematrices", &boneMatrices.GetMatrices( )[ 0 ]( 0, 0 ), boneMatrices.GetMatrices( ).size( ) );

			float scale = actor.getBoundingRadius( ) / playerModelSize;
			float model [] = { scale * rightvec[ 0 ], scale * upvec[ 0 ], scale * forwardvec[ 0 ], actor.getPosition( )[ 0 ],
								scale * rightvec[ 1 ], scale * upvec[ 1 ], scale * forwardvec[ 1 ], 0.0f,
								scale * rightvec[ 2 ], scale * upvec[ 2 ], scale * forwardvec[ 2 ], actor.getPosition( )[ 2 ],
								0.0f, 0.0f, 0.0f, 1.0f };
			shader.setUniform( "model", model );
			playerModel.drawIndexed( GL_TRIANGLES );
		}
	}
}

void GameView::drawBrushes( Shader & shader, const std::string & texparamname, bool usetexture ) {
	for( std::map< std::string, DrawableObject >::iterator iter = brushPolygons.begin( ); iter != brushPolygons.end( ); ++iter ) {
		const std::string & texname = iter->first;
		DrawableObject & drawableobject = iter->second;

		if( normtextures.find( texname ) == normtextures.end( ) ) {
			if( usetexture ) {
				glActiveTexture( GL_TEXTURE0 );
				if( textures.find( texname ) != textures.end( ) ) {
					glBindTexture( GL_TEXTURE_2D, textures[ texname ] );
				} else {
					glBindTexture( GL_TEXTURE_2D, notexTex );
				}
				shader.setUniform( texparamname, 0 );
			}

			drawableobject.bindBuffers( );
			drawableobject.drawIndexed( GL_TRIANGLES );
		}
	}
}

void GameView::drawBrushesNormals( Shader & shader, const std::string & texparamname, bool usetexture ) {
	for( std::map< std::string, DrawableObject >::iterator iter = brushPolygons.begin( ); iter != brushPolygons.end( ); ++iter ) {
		const std::string & texname = iter->first;
		DrawableObject & drawableobject = iter->second;

		if( normtextures.find( texname ) != normtextures.end( ) ) {
			if( usetexture ) {
				glActiveTexture( GL_TEXTURE0 );
				if( textures.find( texname ) != textures.end( ) ) {
					glBindTexture( GL_TEXTURE_2D, textures[ texname ] );
				} else {
					glBindTexture( GL_TEXTURE_2D, notexTex );
				}
				shader.setUniform( texparamname, 0 );
				
				glActiveTexture( GL_TEXTURE1 );
				glBindTexture( GL_TEXTURE_2D, normtextures[ texname ] );
				shader.setUniform( "normalmap", 1 );
			}

			drawableobject.bindBuffers( );
			drawableobject.drawIndexed( GL_TRIANGLES );
		}
	}
}

void GameView::drawItemWeapon( const ItemWeapon * item, float time ) {
	float scale = game->getItemBoundingRadius( );
	float rot1m [] = { cos( time ), 0.0f, -sin( time ), 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					sin( time ), 0.0f, cos( time ), 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float rot2m [] = { cos( time * 2.0f ), -sin( time * 2.0f ), 0.0f, 0.0f,
					sin( time * 2.0f ), cos( time * 2.0f ), 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float scalem [] = { scale, 0.0f, 0.0f, item->getPosition( )[ 0 ],
						0.0f, scale, 0.0f, item->getPosition( )[ 1 ],
						0.0f, 0.0f, scale, item->getPosition( )[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	float model[ 16 ];
	RowMajor< 4, 4, float >( model ) = RowMajor< 4, 4, float >( scalem ) * RowMajor< 4, 4, float >( rot2m ) * RowMajor< 4, 4, float >( rot1m );

	threedtexShader.setUniform( "model", model );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, projTex[ typeid( *item->getWeapon( ).getModel( ) ).name( ) ] );
	threedtexShader.setUniform( "texture", 0 );

	weaponBox.bindBuffers( );
	weaponBox.draw( GL_TRIANGLES );
}

void GameView::drawItemPowerup( const ItemPowerup * item, float time ) {
	float scale = game->getItemBoundingRadius( );
	float rot1m [] = { cos( time ), 0.0f, -sin( time ), 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					sin( time ), 0.0f, cos( time ), 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float rot2m [] = { cos( time * 2.0f ), -sin( time * 2.0f ), 0.0f, 0.0f,
					sin( time * 2.0f ), cos( time * 2.0f ), 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float scalem [] = { scale, 0.0f, 0.0f, item->getPosition( )[ 0 ],
						0.0f, scale, 0.0f, item->getPosition( )[ 1 ],
						0.0f, 0.0f, scale, item->getPosition( )[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	float model[ 16 ];
	RowMajor< 4, 4, float >( model ) = RowMajor< 4, 4, float >( scalem ) * RowMajor< 4, 4, float >( rot2m ) * RowMajor< 4, 4, float >( rot1m );

	threedtexShader.setUniform( "model", model );
	glActiveTexture( GL_TEXTURE0 );
	if( item->getType( ) == "homing" ) {
		glBindTexture( GL_TEXTURE_2D, homingTex );
	} else if( item->getType( ) == "attackspeed" ) {
		glBindTexture( GL_TEXTURE_2D, attackspeedTex );
	} else if( item->getType( ) == "speed" ) {
		glBindTexture( GL_TEXTURE_2D, speedTex );
	} else if( item->getType( ) == "health" ) {
		glBindTexture( GL_TEXTURE_2D, plusTex );
	}
	threedtexShader.setUniform( "texture", 0 );

	weaponBox.bindBuffers( );
	weaponBox.draw( GL_TRIANGLES );
}

void GameView::drawProjectileMelee( const ProjectileMelee * projectile, float time ) {
	float model[ 16 ];
	projIdentity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 1.0f, 1.0f, 1.0f, 0.7f );

	sphereShape.bindBuffers( );
	sphereShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileBullet( const ProjectileBullet * projectile, float time ) {
	float model[ 16 ];
	projStretchVelocity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 0.2f, 0.7f, 1.0f, 1.0f );

	cubeShape.bindBuffers( );
	cubeShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileBounceLaser( const ProjectileBounceLaser * projectile, float time ) {
	float model[ 16 ];
	projStretchVelocity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 1.0f, 0.1f, 0.3f, 1.0f );

	sphereShape.bindBuffers( );
	sphereShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileRocket( const ProjectileRocket * projectile, float time ) {
	float model[ 16 ];
	projStretchVelocity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 1.0f, 0.8f, 0.0f, 1.0f );

	cubeShape.bindBuffers( );
	cubeShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileGrenade( const ProjectileGrenade * projectile, float time ) {
	float model[ 16 ];
	projIdentity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 0.6f, 1.0f, 0.3f, 1.0f );

	sphereShape.bindBuffers( );
	sphereShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileFlame( const ProjectileFlame * projectile, float time ) {
	float t = ( time - projectile->getStartTime( ) ) / projectile->getAliveTime( );
	float radius = projectile->getBoundingRadius( );
	const vec3f & pos = projectile->getPosition( );
	float scale = radius * ( 1.0f + t );
	float model [] = { scale, 0.0f, 0.0f, pos[ 0 ],
						0.0f, scale, 0.0f, pos[ 1 ],
						0.0f, 0.0f, scale, pos[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 1.0f, 0.8f, 0.3f, 0.75f - 0.75f * t * t );

	sphereShape.bindBuffers( );
	sphereShape.draw( GL_TRIANGLES );
}

void GameView::drawProjectileBFG( const ProjectileBFG * projectile, float time ) {
	float model[ 16 ];
	projIdentity( model, projectile );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 0.9f, 0.5f, 0.8f, 1.0f );

	sphereShape.bindBuffers( );
	sphereShape.draw( GL_TRIANGLES );
}

/*void GameView::drawWorldShadowMapped( const float * projection, const float * view, const Light & light, const vec3f & lightdirection, const vec3f & lightup, float fov, float lightatten, int & lightiteration, const ActorModel * actorIgnore ) {
	static const float identitymatrix [] = { 1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f };

	float lightproj[ 16 ];
	perspective( lightproj, fov, 1.0f, 4.0f, 1000.0f );

	float lightview[ 16 ];
	lookat( lightview, light.position[ 0 ], light.position[ 1 ], light.position[ 2 ], light.position[ 0 ] + lightdirection[ 0 ], light.position[ 1 ] + lightdirection[ 1 ], light.position[ 2 ] + lightdirection[ 2 ], lightup[ 0 ], lightup[ 1 ], lightup[ 2 ] );

	static const float bias [] = { 0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, 0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 0.5f, 0.5f,
						0.0f, 0.0f, 0.0f, 1.0f };

	float lightmatrix[ 16 ];
	RowMajor< 4, 4, float >( lightmatrix ) = RowMajor< 4, 4, float >( bias ) * RowMajor< 4, 4, float >( lightproj ) * RowMajor< 4, 4, float >( lightview );

	// render shadow map
	glBindFramebuffer( GL_FRAMEBUFFER, shadowfbo );
	glViewport( 0, 0, 768, 768 );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glDepthMask( GL_TRUE );
	glClear( GL_DEPTH_BUFFER_BIT );
	glDisable( GL_BLEND );
	glCullFace( GL_FRONT );

	plainShader.useProgram( );
	plainShader.setUniform( "projection", lightproj );
	plainShader.setUniform( "view", lightview );
	plainShader.setUniform( "model", identitymatrix );
	drawBrushes( plainShader, "", false );

	plainanimShader.useProgram( );
	plainanimShader.setUniform( "projection", lightproj );
	plainanimShader.setUniform( "view", lightview );
	drawActors( plainanimShader, actorIgnore );

	if( bWriteShadowMaps ) {
		float * depthdata = new float[ 768 * 768 ];
		glReadPixels( 0, 0, 768, 768, GL_DEPTH_COMPONENT, GL_FLOAT, depthdata );
		unsigned char * tgadata = new unsigned char[ 768 * 768 * 3 ];
		for( int i = 0; i < 768 * 768; i++ ) {
			float scaled = 256.0f * pow( depthdata[ i ], 32.0f );
			tgadata[ i * 3 + 0 ] = (unsigned char)scaled;
			tgadata[ i * 3 + 1 ] = (unsigned char)scaled;
			tgadata[ i * 3 + 2 ] = (unsigned char)scaled;
		}
		delete [] depthdata;

		std::ostringstream filename;
		filename << shadowfilename << "_" << lightiteration << ".tga";
		std::ofstream tgastream( filename.str( ).c_str( ), std::ios::binary );
		WriteTGA( tgastream, 768, 768, 24, tgadata );
		tgastream.close( );

		delete [] tgadata;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, threedfbo );
	glViewport( 0, 0, 768, 768 );
	glDrawBuffer( GL_COLOR_ATTACHMENT1 );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glCullFace( GL_BACK );

	if( lightiteration == 0 ) {
		//glDepthMask( GL_TRUE );
		glDisable( GL_BLEND );
	} else if( lightiteration >= 1 ) {
		//glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}
	lightiteration++;

	threedtexlightshadowShader.useProgram( );
	threedtexlightshadowShader.setUniform( "projection", projection );
	threedtexlightshadowShader.setUniform( "view", view );
	threedtexlightshadowShader.setUniform( "model", identitymatrix );
	threedtexlightshadowShader.setUniform( "light", light.position[ 0 ], light.position[ 1 ], light.position[ 2 ], lightatten );
	threedtexlightshadowShader.setUniform( "lightcolor", light.color[ 0 ], light.color[ 1 ], light.color[ 2 ], light.power );
	threedtexlightshadowShader.setUniform( "lightmatrix", lightmatrix );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, shadowDepthRT );
	threedtexlightshadowShader.setUniform( "shadowmap", 1 );

	drawBrushes( threedtexlightshadowShader, "albedo", true );

	threedtexlightshadowanimShader.useProgram( );
	threedtexlightshadowanimShader.setUniform( "projection", projection );
	threedtexlightshadowanimShader.setUniform( "view", view );
	threedtexlightshadowanimShader.setUniform( "light", light.position[ 0 ], light.position[ 1 ], light.position[ 2 ], lightatten );
	threedtexlightshadowanimShader.setUniform( "lightcolor", light.color[ 0 ], light.color[ 1 ], light.color[ 2 ], light.power );
	threedtexlightshadowanimShader.setUniform( "lightmatrix", lightmatrix );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, shadowDepthRT );
	threedtexlightshadowanimShader.setUniform( "shadowmap", 1 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, actorTex );
	threedtexlightshadowanimShader.setUniform( "albedo", 0 );
	drawActors( threedtexlightshadowanimShader, 0 );
}*/

/*void GameView::drawWorldLit( const float * projection, const float * view, const Light & light, float lightatten, int & lightiteration ) {
	static const float identitymatrix [] = { 1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f };

	if( lightiteration == 0 ) {
	//	glDepthMask( GL_TRUE );
		glDisable( GL_BLEND );
	} else if( lightiteration >= 1 ) {
	//	glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}
	lightiteration++;

	threedtexlightShader.useProgram( );
	threedtexlightShader.setUniform( "projection", projection );
	threedtexlightShader.setUniform( "view", view );
	threedtexlightShader.setUniform( "light", light.position[ 0 ], light.position[ 1 ], light.position[ 2 ], lightatten );
	threedtexlightShader.setUniform( "lightcolor", light.color[ 0 ], light.color[ 1 ], light.color[ 2 ], light.power );
	threedtexlightShader.setUniform( "model", identitymatrix );
	drawBrushes( threedtexlightShader, "albedo", true );

	threedtexlightanimShader.useProgram( );
	threedtexlightanimShader.setUniform( "projection", projection );
	threedtexlightanimShader.setUniform( "view", view );
	threedtexlightanimShader.setUniform( "light", light.position[ 0 ], light.position[ 1 ], light.position[ 2 ], lightatten );
	threedtexlightanimShader.setUniform( "lightcolor", light.color[ 0 ], light.color[ 1 ], light.color[ 2 ], light.power );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, actorTex );
	threedtexlightanimShader.setUniform( "albedo", 0 );
	drawActors( threedtexlightanimShader, 0 );
}*/

void GameView::drawHealthBar( const ActorModel & actor ) {
	if( actor.getHealth( ) > 0.0f ) {
		{
			float xoffset = ( 1.0f - actor.getHealth( ) / 100.0f ) * actor.getBoundingRadius( );
			float model [] = { xoffset, 0.0f, 0.0f, actor.getPosition( )[ 0 ] + ( actor.getBoundingRadius( ) - xoffset ),
								0.0f, 1.0f, 0.0f, actor.getPosition( )[ 1 ] + actor.getBoundingRadius( ),
								0.0f, 0.0f, 1.0f, actor.getPosition( )[ 2 ] - actor.getBoundingRadius( ),
								0.0f, 0.0f, 0.0f, 1.0f };

			projShader.setUniform( "model", model );
			projShader.setUniform( "color", 1.0f, 0.0f, 0.0f, 1.0f );

			cubeShape.draw( GL_TRIANGLES );
		}

		{
			float xoffset = actor.getHealth( ) / 100.0f * actor.getBoundingRadius( );
			float model [] = { xoffset, 0.0f, 0.0f, actor.getPosition( )[ 0 ] - ( actor.getBoundingRadius( ) - xoffset ),
								0.0f, 1.0f, 0.0f, actor.getPosition( )[ 1 ] + actor.getBoundingRadius( ),
								0.0f, 0.0f, 1.0f, actor.getPosition( )[ 2 ] - actor.getBoundingRadius( ),
								0.0f, 0.0f, 0.0f, 1.0f };

			projShader.setUniform( "model", model );
			projShader.setUniform( "color", 0.0f, 1.0f, 0.0f, 1.0f );

			cubeShape.draw( GL_TRIANGLES );
		}
	}
}

void GameView::drawDyingActor( const ActorModel & actor, float time ) {
	vec3f upvec = Vector3( 0.0f, 1.0f, 0.0f );
	const vec3f & forwardvec = actor.getOrientation( );
	vec3f rightvec = CrossProduct( upvec, forwardvec );

	if( actor.getHealth( ) > 0.0f && ( time - actor.getDeathTime( ) ) < 0.25f ) {
		float t = ( time - actor.getDeathTime( ) ) / 0.25f;
		float scale = 2.0f * ( 2.0f - t ) * actor.getBoundingRadius( ) / playerModelSize;
		float alpha = t;
		float model [] = { scale * rightvec[ 0 ], scale * upvec[ 0 ], scale * forwardvec[ 0 ], actor.getPosition( )[ 0 ],
							scale * rightvec[ 1 ], scale * upvec[ 1 ], scale * forwardvec[ 1 ], actor.getPosition( )[ 1 ],
							scale * rightvec[ 2 ], scale * upvec[ 2 ], scale * forwardvec[ 2 ], actor.getPosition( )[ 2 ],
							0.0f, 0.0f, 0.0f, 1.0f };
		projShader.setUniform( "model", model );
		projShader.setUniform( "color", 1.0f, 1.0f, 1.0f, alpha );
		playerModel.drawIndexed( GL_TRIANGLES );
	} else if( actor.getHealth( ) <= 0.0f && ( time - actor.getDeathTime( ) ) < 0.25f ) {
		float scale = ( time - actor.getDeathTime( ) + 1.0f ) * 4.0f * actor.getBoundingRadius( ) / playerModelSize;
		float alpha = 1.0f - ( time - actor.getDeathTime( ) ) / 0.25f;
		float model [] = { scale * rightvec[ 0 ], scale * upvec[ 0 ], scale * forwardvec[ 0 ], actor.getPosition( )[ 0 ],
							scale * rightvec[ 1 ], scale * upvec[ 1 ], scale * forwardvec[ 1 ], actor.getPosition( )[ 1 ],
							scale * rightvec[ 2 ], scale * upvec[ 2 ], scale * forwardvec[ 2 ], actor.getPosition( )[ 2 ],
							0.0f, 0.0f, 0.0f, 1.0f };
		projShader.setUniform( "model", model );
		projShader.setUniform( "color", 1.0f, 1.0f, 1.0f, alpha );
		playerModel.drawIndexed( GL_TRIANGLES );
	}
}

void GameView::drawExplosionSphere( const Particle & explosion, float time ) {
	float scale = ( time - explosion.starttime ) / explosion.keepalive;

	float model [] = { scale * 96.0f, 0.0f, 0.0f, explosion.position[ 0 ],
						0.0f, scale * 96.0f, 0.0f, explosion.position[ 1 ],
						0.0f, 0.0f, scale * 96.0f, explosion.position[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 1.0f, 0.5f, 0.0f, 0.25f - scale * scale * scale * scale * 0.25f );

	sphereShape.draw( GL_TRIANGLES );
}

void GameView::drawItemEffect( const Particle & itemparticle, float time ) {
	float scale = ( time - itemparticle.starttime ) / itemparticle.keepalive;
	float rot1m [] = { cos( time ), 0.0f, -sin( time ), 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					sin( time ), 0.0f, cos( time ), 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float rot2m [] = { cos( time * 2.0f ), -sin( time * 2.0f ), 0.0f, 0.0f,
					sin( time * 2.0f ), cos( time * 2.0f ), 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f };
	float scalem [] = { ( scale * 4.0f + 1.0f ) * game->getItemBoundingRadius( ), 0.0f, 0.0f, itemparticle.position[ 0 ],
						0.0f, ( scale * 4.0f + 1.0f ) * game->getItemBoundingRadius( ), 0.0f, itemparticle.position[ 1 ],
						0.0f, 0.0f, ( scale * 4.0f + 1.0f ) * game->getItemBoundingRadius( ), itemparticle.position[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	float model[ 16 ];
	RowMajor< 4, 4, float >( model ) = RowMajor< 4, 4, float >( scalem ) * RowMajor< 4, 4, float >( rot2m ) * RowMajor< 4, 4, float >( rot1m );

	projShader.setUniform( "model", model );
	projShader.setUniform( "color", 0.0f, 0.0f, 0.0f, 1.0f - scale );

	cubeShape.draw( GL_TRIANGLES );
}

void GameView::writeDepthMap( const std::string & suffix, int size, float exponent ) {
	float * depthdata = new float[ size * size ];
	glReadPixels( 0, 0, size, size, GL_DEPTH_COMPONENT, GL_FLOAT, depthdata );
	unsigned char * tgadata = new unsigned char[ size * size * 3 ];
	for( int i = 0; i < size * size; i++ ) {
		float scaled = 256.0f * pow( depthdata[ i ], exponent );
		tgadata[ i * 3 + 0 ] = (unsigned char)scaled;
		tgadata[ i * 3 + 1 ] = (unsigned char)scaled;
		tgadata[ i * 3 + 2 ] = (unsigned char)scaled;
	}
	delete [] depthdata;

	std::ostringstream filename;
	filename << shadowfilename << "_" << suffix << ".tga";
	std::ofstream tgastream( filename.str( ).c_str( ), std::ios::binary );
	WriteTGA( tgastream, size, size, 24, tgadata );
	tgastream.close( );

	delete [] tgadata;
}

void GameView::renderShadowMap( const float * lightview, const float * lightproj, const ActorModel * actorIgnore ) {
	static const float identitymatrix [] = { 1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f };
	// render shadow map
	glBindFramebuffer( GL_FRAMEBUFFER, shadowfbo );
	glViewport( 0, 0, shadowfboSize, shadowfboSize );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glDepthMask( GL_TRUE );
	glEnable( GL_DEPTH_TEST );
	glClear( GL_DEPTH_BUFFER_BIT );
	glDisable( GL_BLEND );
	glCullFace( GL_FRONT );
	glDepthFunc( GL_LEQUAL );

	plainShader.useProgram( );
	plainShader.setUniform( "projection", lightproj );
	plainShader.setUniform( "view", lightview );
	plainShader.setUniform( "model", identitymatrix );
	drawBrushes( plainShader, "", false );
	drawBrushesNormals( plainShader, "", false );

	plainanimShader.useProgram( );
	plainanimShader.setUniform( "projection", lightproj );
	plainanimShader.setUniform( "view", lightview );
	drawActors( plainanimShader, actorIgnore );
}

void GameView::drawDeferredDirectionallight( const DirectionalLight & directionallight, const Matrix< 4, 4, float > & viewprojInv, int & lightiteration ) {
	vec3f lightup = Norm( CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), directionallight.direction ) ) < 1.0f / 32.0f ? CrossProduct( Vector3( 0.0f, 0.0f, 1.0f ), directionallight.direction ) : CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), directionallight.direction );
	vec3f lightright = CrossProduct( directionallight.direction, lightup );

	vec3f lightpos;
	float lightproj[ 16 ];
	{
		vec3f camerapos = getViewPos( ), cameradir = getViewDir( );
		vec3f cameraup = Norm( CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), cameradir ) ) < 1.0f / 32.0f ? CrossProduct( Vector3( 0.0f, 0.0f, 1.0f ), cameradir ) : CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), cameradir );
		vec3f cameraright = CrossProduct( cameradir, cameraup );
		float cnear = 1.0f, cfar = 1024.0f;

		float hnear = 2.0f * tan( ( 3.1415926f / 3.0f ) / 2.0f ) * cnear;
		float wnear = hnear;
		float hfar = 2.0f * tan( ( 3.1415926f / 3.0f ) / 2.0f ) * cfar;
		float wfar = hfar;

		vec3f frustumpts [] = { camerapos + cameradir * cnear + cameraright * wnear * -0.5f + cameraup * hnear * -0.5f,
								camerapos + cameradir * cnear + cameraright * wnear * 0.5f + cameraup * hnear * -0.5f,
								camerapos + cameradir * cnear + cameraright * wnear * -0.5f + cameraup * hnear * 0.5f,
								camerapos + cameradir * cnear + cameraright * wnear * 0.5f + cameraup * hnear * 0.5f,
								camerapos + cameradir * cfar + cameraright * wfar * -0.5f + cameraup * hfar * -0.5f,
								camerapos + cameradir * cfar + cameraright * wfar * 0.5f + cameraup * hfar * -0.5f,
								camerapos + cameradir * cfar + cameraright * wfar * -0.5f + cameraup * hfar * 0.5f,
								camerapos + cameradir * cfar + cameraright * wfar * 0.5f + cameraup * hfar * 0.5f };

		vec3f fminx = *std::min_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, lightright ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, lightright ) );
		vec3f fmaxx = *std::max_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, lightright ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, lightright ) );
		vec3f fminy = *std::min_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, lightup ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, lightup ) );
		vec3f fmaxy = *std::max_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, lightup ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, lightup ) );
		vec3f fminz = *std::min_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, directionallight.direction ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, directionallight.direction ) );
		vec3f fmaxz = *std::max_element( frustumpts, frustumpts + 8, boost::bind( DotProduct< Matrix, Matrix, 3, float >, _1, directionallight.direction ) < boost::bind( DotProduct< Matrix, Matrix, 3, float >, _2, directionallight.direction ) );

		float nearpdist = DotProduct( directionallight.direction, fminz );
		float farpdist = DotProduct( directionallight.direction, fmaxz );
		float leftpdist = DotProduct( lightright, fminx );
		float rightpdist = DotProduct( lightright, fmaxx );
		float downpdist = DotProduct( lightup, fminy );
		float uppdist = DotProduct( lightup, fmaxy );

		vec3f centerright = lightright * ( rightpdist - leftpdist ) * 0.5f + lightright * leftpdist;
		vec3f centerup = lightup * ( uppdist - downpdist ) * 0.5f + lightup * downpdist;

		lightpos = directionallight.direction * nearpdist + centerright + centerup;

		float fwidth = rightpdist - leftpdist;
		float fheight = uppdist - downpdist;

		orthographic( lightproj, -fwidth / 2.0f, fwidth / 2.0f, -fheight / 2.0f, fheight / 2.0f, 0.0f, farpdist - nearpdist );
	}

	float lightview[ 16 ];
	lookat( lightview, lightpos[ 0 ], lightpos[ 1 ], lightpos[ 2 ], 
						lightpos[ 0 ] + directionallight.direction[ 0 ], lightpos[ 1 ] + directionallight.direction[ 1 ], lightpos[ 2 ] + directionallight.direction[ 2 ], 
						lightup[ 0 ], lightup[ 1 ], lightup[ 2 ] );

	static const float bias [] = { 0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, 0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 0.5f, 0.5f,
						0.0f, 0.0f, 0.0f, 1.0f };

	float lightmatrix[ 16 ];
	RowMajor< 4, 4, float >( lightmatrix ) = RowMajor< 4, 4, float >( bias ) * RowMajor< 4, 4, float >( lightproj ) * RowMajor< 4, 4, float >( lightview );

	renderShadowMap( lightview, lightproj, 0 );
	if( bWriteShadowMaps ) {
		std::ostringstream stream;
		stream << lightiteration;
		writeDepthMap( stream.str( ), shadowfboSize, 1.0f );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, 768, 768 );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	//glCullFace( GL_BACK );
	glCullFace( GL_FRONT );
	//glDisable( GL_CULL_FACE );
	//glDepthFunc( GL_GEQUAL );
	glDisable( GL_DEPTH_TEST );

	if( lightiteration == 0 ) {
		glDisable( GL_BLEND );
	} else {
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}
	lightiteration++;

	float identity [] = { 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f };
	gdirlightShader.useProgram( );
	gdirlightShader.setUniform( "projection", identity );
	gdirlightShader.setUniform( "view", identity );
	gdirlightShader.setUniform( "viewprojInv", &viewprojInv( 0, 0 ) );
	gdirlightShader.setUniform( "lightmatrix", lightmatrix );
	gdirlightShader.setUniform( "albedotex", 0 );
	gdirlightShader.setUniform( "normaltex", 1 );
	gdirlightShader.setUniform( "depthtex", 2 );
	gdirlightShader.setUniform( "shadowmap", 3 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, galbedoRT );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, gnormalRT );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, gdepthRT );
	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, shadowDepthRT );

	float model [] = { 2.0f, 0.0f, 0.0f, -1.0f,
						0.0f, -2.0f, 0.0f, 1.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f };
	gdirlightShader.setUniform( "model", model );
	//gdirlightShader.setUniform( "lightpos", lightpos[ 0 ], lightpos[ 1 ], lightpos[ 2 ], 0.0f );
	gdirlightShader.setUniform( "lightdir", directionallight.direction[ 0 ], directionallight.direction[ 1 ], directionallight.direction[ 2 ] );
	gdirlightShader.setUniform( "lightcolor", directionallight.color[ 0 ], directionallight.color[ 1 ], directionallight.color[ 2 ], directionallight.power );

	hudquad.bindBuffers( );
	hudquad.draw( GL_TRIANGLES );

	//glEnable( GL_CULL_FACE );
	//glEnable( GL_DEPTH_TEST );
}

void GameView::drawDeferredSpotlight( const SpotLight & spotlight, float * projection, float * view, const Matrix< 4, 4, float > & viewprojInv, const ActorModel * actorIgnore, int & lightiteration ) {
	vec3f lightup = Norm( CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), spotlight.direction ) ) < 1.0f / 32.0f ? CrossProduct( Vector3( 0.0f, 0.0f, 1.0f ), spotlight.direction ) : CrossProduct( Vector3( 0.0f, 1.0f, 0.0f ), spotlight.direction );
	vec3f lightright = CrossProduct( spotlight.direction, lightup );

	float lightproj[ 16 ];
	perspective( lightproj, spotlight.fov, 1.0f, 4.0f, 1000.0f );
	//orthographic( lightproj, -256.0f, 256.0f, -256.0f, 256.0f, 0.0f, 1024.0f );

	float lightview[ 16 ];
	lookat( lightview, spotlight.position[ 0 ], spotlight.position[ 1 ], spotlight.position[ 2 ], 
						spotlight.position[ 0 ] + spotlight.direction[ 0 ], spotlight.position[ 1 ] + spotlight.direction[ 1 ], spotlight.position[ 2 ] + spotlight.direction[ 2 ], 
						lightup[ 0 ], lightup[ 1 ], lightup[ 2 ] );

	static const float bias [] = { 0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, 0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 0.5f, 0.5f,
						0.0f, 0.0f, 0.0f, 1.0f };

	float lightmatrix[ 16 ];
	RowMajor< 4, 4, float >( lightmatrix ) = RowMajor< 4, 4, float >( bias ) * RowMajor< 4, 4, float >( lightproj ) * RowMajor< 4, 4, float >( lightview );

	renderShadowMap( lightview, lightproj, actorIgnore );
	if( bWriteShadowMaps ) {
		std::ostringstream stream;
		stream << lightiteration;
		writeDepthMap( stream.str( ), shadowfboSize, 32.0f );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, 768, 768 );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glCullFace( GL_BACK );
	//glDepthFunc( GL_GEQUAL );
	glDisable( GL_DEPTH_TEST );

	if( lightiteration == 0 ) {
		glDisable( GL_BLEND );
	} else {
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}
	lightiteration++;

	gspotlightShader.useProgram( );
	gspotlightShader.setUniform( "projection", projection );
	gspotlightShader.setUniform( "view", view );
	gspotlightShader.setUniform( "viewprojInv", &viewprojInv( 0, 0 ) );
	gspotlightShader.setUniform( "lightmatrix", lightmatrix );
	gspotlightShader.setUniform( "albedotex", 0 );
	gspotlightShader.setUniform( "normaltex", 1 );
	gspotlightShader.setUniform( "depthtex", 2 );
	gspotlightShader.setUniform( "shadowmap", 3 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, galbedoRT );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, gnormalRT );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, gdepthRT );
	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, shadowDepthRT );

	//float radius = 512.0f;
	float radius = sqrt( ( 4.0f * spotlight.power - 1.0f ) / spotlight.atten );
	float model [] = { radius * lightright[ 0 ], radius * lightup[ 0 ], radius * spotlight.direction[ 0 ], spotlight.position[ 0 ],
						radius * lightright[ 1 ], radius * lightup[ 1 ], radius * spotlight.direction[ 1 ], spotlight.position[ 1 ],
						radius * lightright[ 2 ], radius * lightup[ 2 ], radius * spotlight.direction[ 2 ], spotlight.position[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	gspotlightShader.setUniform( "model", model );
	gspotlightShader.setUniform( "lightpos", spotlight.position[ 0 ], spotlight.position[ 1 ], spotlight.position[ 2 ], spotlight.atten );
	gspotlightShader.setUniform( "lightcolor", spotlight.color[ 0 ], spotlight.color[ 1 ], spotlight.color[ 2 ], spotlight.power );

	coneShape.bindBuffers( );
	coneShape.draw( GL_TRIANGLES );
}

void GameView::drawDeferredPointlight( const PointLight & pointlight, int & lightiteration ) {
	if( lightiteration == 0 ) {
		glDisable( GL_BLEND );
	} else {
		glEnable( GL_BLEND );
		glBlendFunc( GL_ONE, GL_ONE );
	}
	lightiteration++;

	//float radius = 256.0f;
	float radius = sqrt( ( 4.0f * pointlight.power - 1.0f ) / pointlight.atten );
	float model [] = { radius, 0.0f, 0.0f, pointlight.position[ 0 ],
						0.0f, radius, 0.0f, pointlight.position[ 1 ],
						0.0f, 0.0f, radius, pointlight.position[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	gpointlightShader.setUniform( "model", model );
	gpointlightShader.setUniform( "lightpos", pointlight.position[ 0 ], pointlight.position[ 1 ], pointlight.position[ 2 ], pointlight.atten );
	gpointlightShader.setUniform( "lightcolor", pointlight.color[ 0 ], pointlight.color[ 1 ], pointlight.color[ 2 ], pointlight.power );

	sphereShape.draw( GL_TRIANGLES );
}

// deferred rendering
void GameView::draw( float time ) {
	float projection[ 16 ];
	getProjectionMatrix( projection );

	float view[ 16 ];
	getViewMatrix( view );

	Matrix< 4, 4, float > viewproj = RowMajor< 4, 4, float >( projection ) * RowMajor< 4, 4, float >( view );
	Matrix< 4, 4, float > viewprojInv;
	Sub< 4, 1 >::Matrix( viewprojInv, 0, 0 ) = Solve( viewproj, Vector4( 1.0f, 0.0f, 0.0f, 0.0f ) );
	Sub< 4, 1 >::Matrix( viewprojInv, 0, 1 ) = Solve( viewproj, Vector4( 0.0f, 1.0f, 0.0f, 0.0f ) );
	Sub< 4, 1 >::Matrix( viewprojInv, 0, 2 ) = Solve( viewproj, Vector4( 0.0f, 0.0f, 1.0f, 0.0f ) );
	Sub< 4, 1 >::Matrix( viewprojInv, 0, 3 ) = Solve( viewproj, Vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	static const float identitymatrix [] = { 1.0f, 0.0f, 0.0f, 0.0f,
											0.0f, 1.0f, 0.0f, 0.0f,
											0.0f, 0.0f, 1.0f, 0.0f,
											0.0f, 0.0f, 0.0f, 1.0f };

	// geometry pass
	glBindFramebuffer( GL_FRAMEBUFFER, gbufferfbo );
	GLenum bufferlist [] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, bufferlist );

	glViewport( 0, 0, 768, 768 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDepthMask( GL_TRUE );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	gthreedShader.useProgram( );
	gthreedShader.setUniform( "projection", projection );
	gthreedShader.setUniform( "view", view );
	gthreedShader.setUniform( "model", identitymatrix );

	drawBrushes( gthreedShader, "albedo", true );

	gthreednormmapShader.useProgram( );
	gthreednormmapShader.setUniform( "projection", projection );
	gthreednormmapShader.setUniform( "view", view );
	gthreednormmapShader.setUniform( "model", identitymatrix );

	drawBrushesNormals( gthreednormmapShader, "albedo", true );

	gthreedanimShader.useProgram( );
	gthreedanimShader.setUniform( "projection", projection );
	gthreedanimShader.setUniform( "view", view );
	gthreedanimShader.setUniform( "albedo", 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, actorTex );

	drawActors( gthreedanimShader, 0 );

	// lighting pass
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	glViewport( 0, 0, 768, 768 );
	//glClearDepth( 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//glClearDepth( 1.0f );

	//glDepthMask( GL_FALSE );
	//glDisable( GL_DEPTH_TEST );

	int lightiteration = 0;

	{
		glCullFace( GL_FRONT );
		glDepthFunc( GL_LEQUAL );

		glDisable( GL_BLEND );

		float identity [] = { 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f };
		gambientShader.useProgram( );
		gambientShader.setUniform( "projection", identity );
		gambientShader.setUniform( "view", identity );
		gambientShader.setUniform( "albedotex", 0 );
		gambientShader.setUniform( "normaltex", 1 );
		gambientShader.setUniform( "depthtex", 2 );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, galbedoRT );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, gnormalRT );
		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D, gdepthRT );

		float model [] = { 2.0f, 0.0f, 0.0f, -1.0f,
							0.0f, -2.0f, 0.0f, 1.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f };
		gambientShader.setUniform( "model", model );

		hudquad.bindBuffers( );
		hudquad.draw( GL_TRIANGLES );

		lightiteration++;
	}

	glDisable( GL_DEPTH_TEST );

	for( std::list< DirectionalLight >::const_iterator iter = directionallights.begin( ); iter != directionallights.end( ); ++iter ) {
		drawDeferredDirectionallight( *iter, viewprojInv, lightiteration );
	}

	for( std::list< SpotLight >::const_iterator iter = spotlights.begin( ); iter != spotlights.end( ); ++iter ) {
		drawDeferredSpotlight( *iter, projection, view, viewprojInv, 0, lightiteration );
	}

	for( std::list< PointLight >::const_iterator iter = pointlights.begin( ); iter != pointlights.end( ); ++iter ) {
		for( int i = 0; i < 6; i++ ) {
			vec3f lightdir = Vector3( i == 0 ? 1.0f : ( i == 1 ? -1.0f : 0.0f ), i == 2 ? 1.0f : ( i == 3 ? -1.0f : 0.0f ), i == 4 ? 1.0f : ( i == 5 ? -1.0f : 0.0f ) );
			SpotLight spotlight( iter->position, lightdir, iter->color, 3.1415926f / 2.0f, iter->power, iter->atten );
			drawDeferredSpotlight( spotlight, projection, view, viewprojInv, 0, lightiteration );
		}
	}

	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		if( iter->getHealth( ) > 0.0f ) {
			drawDeferredSpotlight( SpotLight( iter->getPosition( ), iter->getOrientation( ), Vector3( 1.0f, 1.0f, 1.0f ), 3.1415926f / 2.0f, 4.0f, 1.0f / 4096.0f ), projection, view, viewprojInv, &(*iter), lightiteration );
		}
	}

	if( bWriteShadowMaps ) {
		writeDepthMap( "depth", 768, 32.0f );
	}

	bWriteShadowMaps = false;

	gpointlightShader.useProgram( );
	gpointlightShader.setUniform( "projection", projection );
	gpointlightShader.setUniform( "view", view );
	gpointlightShader.setUniform( "viewprojInv", &viewprojInv( 0, 0 ) );
	gpointlightShader.setUniform( "albedotex", 0 );
	gpointlightShader.setUniform( "normaltex", 1 );
	gpointlightShader.setUniform( "depthtex", 2 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, galbedoRT );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, gnormalRT );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, gdepthRT );

	glCullFace( GL_FRONT );
	
	sphereShape.bindBuffers( );
	// explosion lighting
	for( std::list< Particle >::iterator iter = explosions.begin( ); iter != explosions.end( ); ++iter ) {
		Particle & particle = *iter;
		float power = 1.0f - ( time - particle.starttime ) / particle.keepalive;
		drawDeferredPointlight( PointLight( particle.position, Vector3( 1.0f, 0.8f, 0.4f ), 8.0f * power, 1.0f / 1024.0f ), lightiteration );
	}

	// muzzle flares
	for( std::map< const ActorModel *, float >::const_iterator iter = muzzleFlares.begin( ); iter != muzzleFlares.end( ); ++iter ) {
		const ActorModel * actor = iter->first;
		float starttime = iter->second;

		if( time - starttime < 1.0f / 16.0f ) {

			vec3f pos = actor->getPosition( ) + actor->getOrientation( ) * actor->getBoundingRadius( );
			float power = 1.0f - ( time - starttime ) / 0.0625f;
			drawDeferredPointlight( PointLight( pos, Vector3( 1.0f, 0.8f, 0.4f ), 8.0f * power, 1.0f / 1024.0f ), lightiteration );
		}
	}
	
	glEnable( GL_DEPTH_TEST );
	//glDepthMask( GL_TRUE );
	//glDepthFunc( GL_LEQUAL );
	glCullFace( GL_BACK );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// projectiles/particles
	{
		projShader.useProgram( );
		projShader.setUniform( "projection", projection );
		projShader.setUniform( "view", view );

		//std::vector< Projectile * > projcopy( game->getProjectiles( ).begin( ), game->getProjectiles( ).end( ) );
		//std::sort( projcopy.begin( ), projcopy.end( ), ProjCmp );
		//for( std::vector< Projectile * >::const_iterator iter = projcopy.begin( ); iter != projcopy.end( ); ++iter ) {
		// draw projectiles
		if( drawProjectiles ) {
			for( std::list< Projectile * >::const_iterator iter = game->getProjectiles( ).begin( ); iter != game->getProjectiles( ).end( ); ++iter ) {
				const Projectile * projectile = *iter;

				(*projDrawFuncs[ typeid( *projectile ).name( ) ])( this, projectile, time );
			}
		}

		if( drawParticles ) {
			// draw particles
			sphereShape.bindBuffers( );
			for( std::list< Particle >::iterator iter = particles.begin( ); iter != particles.end( ); ++iter ) {
				Particle & particle = *iter;

				float scale = ( time - particle.starttime ) / particle.keepalive;

				float model [] = { 1.0f, 0.0f, 0.0f, particle.position[ 0 ],
									0.0f, 1.0f, 0.0f, particle.position[ 1 ],
									0.0f, 0.0f, 1.0f, particle.position[ 2 ],
									0.0f, 0.0f, 0.0f, 1.0f };

				projShader.setUniform( "model", model );
				projShader.setUniform( "color", 1.0f, 0.7f, 0.3f, 1.0f - scale * scale * scale * scale );

				sphereShape.draw( GL_TRIANGLES );
			}

			// draw blood particles
			for( std::list< Particle >::iterator iter = bloodparticles.begin( ); iter != bloodparticles.end( ); ++iter ) {
				Particle & particle = *iter;

				float scale = ( time - particle.starttime ) / particle.keepalive;

				float model [] = { 1.0f, 0.0f, 0.0f, particle.position[ 0 ],
									0.0f, 1.0f, 0.0f, particle.position[ 1 ],
									0.0f, 0.0f, 1.0f, particle.position[ 2 ],
									0.0f, 0.0f, 0.0f, 1.0f };

				projShader.setUniform( "model", model );
				projShader.setUniform( "color", 0.8f, 0.1f, 0.1f, 1.0f - scale * scale * scale * scale );

				sphereShape.draw( GL_TRIANGLES );
			}
		}
	}

	// items, health bars, misc, etc.,
	{
		glDisable( GL_BLEND );

		threedtexShader.useProgram( );
		threedtexShader.setUniform( "projection", projection );
		threedtexShader.setUniform( "view", view );
		threedtexShader.setUniform( "texscale", 1.0f );

		// items
		for( std::list< Item * >::const_iterator iter = game->getItems( ).begin( ); iter != game->getItems( ).end( ); ++iter ) {
			const Item * item = *iter;

			(*itemDrawFuncs[ typeid( *item ).name( ) ])( this, item, time );
		}

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		//glBlendFunc( GL_SRC_ALPHA, GL_ONE );

		projShader.useProgram( );
		projShader.setUniform( "projection", projection );
		projShader.setUniform( "view", view );

		// health bars
		if( !firstperson ) {
			cubeShape.bindBuffers( );
			for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
				const ActorModel & actor = *iter;

				drawHealthBar( actor );
			}
		}

		// draw spawning/dying actors
		playerModel.bindBuffers( );
		for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
			const ActorModel & actor = *iter;

			drawDyingActor( actor, time );
		}

		// draw explosion spheres
		sphereShape.bindBuffers( );
		for( std::list< Particle >::iterator iter = explosions.begin( ); iter != explosions.end( ); ++iter ) {
			Particle & explosion = *iter;

			drawExplosionSphere( explosion, time );
		}

		// draw item particles
		cubeShape.bindBuffers( );
		for( std::list< Particle >::iterator iter = itemparticles.begin( ); iter != itemparticles.end( ); ++iter ) {
			Particle & itemparticle = *iter;

			drawItemEffect( itemparticle, time );
		}
	}
}

void GameView::projStretchVelocity( float * matrix, const Projectile * projectile ) {
	float speed = Norm( projectile->getVelocity( ) );
	vec3f forward = Normalize( projectile->getVelocity( ) );
	vec3f up = Vector3( 0.0f, 1.0f, 0.0f );
	vec3f right = Normalize( CrossProduct( up, forward ) );
	vec3f newup = Normalize( CrossProduct( forward, right ) );
	float radius = projectile->getBoundingRadius( );

	float model [] = { radius * right[ 0 ], radius * newup[ 0 ], speed * forward[ 0 ], projectile->getPosition( )[ 0 ],
						radius * right[ 1 ], radius * newup[ 1 ], speed * forward[ 1 ], projectile->getPosition( )[ 1 ],
						radius * right[ 2 ], radius * newup[ 2 ], speed * forward[ 2 ], projectile->getPosition( )[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	RowMajor< 4, 4, float >( matrix ) = RowMajor< 4, 4, float >( model ) * 1.0f;
}

void GameView::projIdentity( float * matrix, const Projectile * projectile ) {
	const vec3f & position = projectile->getPosition( );
	float radius = projectile->getBoundingRadius( );
	float model [] = { radius, 0.0f, 0.0f, position[ 0 ],
						0.0f, radius, 0.0f, position[ 1 ],
						0.0f, 0.0f, radius, position[ 2 ],
						0.0f, 0.0f, 0.0f, 1.0f };
	RowMajor< 4, 4, float >( matrix ) = RowMajor< 4, 4, float >( model ) * 1.0f;
}

void GameView::drawItemWeapon( GameView * gameview, const Item * item, float time ) {
	gameview->drawItemWeapon( dynamic_cast< const ItemWeapon * >( item ), time );
}

void GameView::drawItemPowerup( GameView * gameview, const Item * item, float time ) {
	gameview->drawItemPowerup( dynamic_cast< const ItemPowerup * >( item ), time );
}

void GameView::drawProjectileMelee( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileMelee( dynamic_cast< const ProjectileMelee * >( projectile ), time );
}

void GameView::drawProjectileBullet( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileBullet( dynamic_cast< const ProjectileBullet * >( projectile ), time );
}

void GameView::drawProjectileBounceLaser( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileBounceLaser( dynamic_cast< const ProjectileBounceLaser * >( projectile ), time );
}

void GameView::drawProjectileRocket( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileRocket( dynamic_cast< const ProjectileRocket * >( projectile ), time );
}

void GameView::drawProjectileGrenade( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileGrenade( dynamic_cast< const ProjectileGrenade * >( projectile ), time );
}

void GameView::drawProjectileFlame( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileFlame( dynamic_cast< const ProjectileFlame * >( projectile ), time );
}

void GameView::drawProjectileBFG( GameView * gameview, const Projectile * projectile, float time ) {
	gameview->drawProjectileBFG( dynamic_cast< const ProjectileBFG * >( projectile ), time );
}
