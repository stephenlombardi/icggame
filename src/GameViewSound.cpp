#include "GameViewSound.h"

void GameViewSound::init( ) {
	shotSound = alutCreateBufferFromFile( "./sounds/shot2.wav" );
	deathSound = alutCreateBufferFromFile( "./sounds/death2.wav" );
	damageSound = alutCreateBufferFromFile( "./sounds/damage.wav" );
	explosionSound = alutCreateBufferFromFile( "./sounds/explosion2.wav" );
	itemSound = alutCreateBufferFromFile( "./sounds/item2.wav" );

	explosionSources = std::vector< ALuint >( 8 );
	for( size_t i = 0; i < explosionSources.size( ); i++ ) {
		alGenSources( 1, &explosionSources[ i ] );
		alSourcef( explosionSources[ i ], AL_PITCH, 1.0f );
		alSourcef( explosionSources[ i ], AL_GAIN, 1.0f );
		alSourcei( explosionSources[ i ], AL_LOOPING, false );
	}
	explosionIndex = 0;

	music.open( "./sounds/music.ogg" );
	//music.play( );

	ALfloat listenerposition [] = { 0.0f, 1.0f, 0.0f };
	ALfloat listenervelocity [] = { 0.0f, 0.0f, 0.0f };
	ALfloat listenerorientation [] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f };

	alListenerfv( AL_POSITION, listenerposition );
	alListenerfv( AL_VELOCITY, listenervelocity );
	alListenerfv( AL_ORIENTATION, listenerorientation );
}

/*void GameViewSound::initActors( ) {
	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;

		alGenSources( 1, &actorShotSources[ actor.getName( ) ] );
		alSourcef( actorShotSources[ actor.getName( ) ], AL_PITCH, 1.0f );
		alSourcef( actorShotSources[ actor.getName( ) ], AL_GAIN, 1.0f );
		alSourcei( actorShotSources[ actor.getName( ) ], AL_LOOPING, false );

		alGenSources( 1, &actorDeathSources[ actor.getName( ) ] );
		alSourcef( actorDeathSources[ actor.getName( ) ], AL_PITCH, 1.0f );
		alSourcef( actorDeathSources[ actor.getName( ) ], AL_GAIN, 1.0f );
		alSourcei( actorDeathSources[ actor.getName( ) ], AL_LOOPING, false );

		alGenSources( 1, &actorDamageSources[ actor.getName( ) ] );
		alSourcef( actorDamageSources[ actor.getName( ) ], AL_PITCH, 1.0f );
		alSourcef( actorDamageSources[ actor.getName( ) ], AL_GAIN, 1.0f );
		alSourcei( actorDamageSources[ actor.getName( ) ], AL_LOOPING, false );

		alGenSources( 1, &actorItemSources[ actor.getName( ) ] );
		alSourcef( actorItemSources[ actor.getName( ) ], AL_PITCH, 1.0f );
		alSourcef( actorItemSources[ actor.getName( ) ], AL_GAIN, 1.0f );
		alSourcei( actorItemSources[ actor.getName( ) ], AL_LOOPING, false );
	}
}*/

void GameViewSound::uninit( ) {
	music.uninit( );

	for( size_t i = 0; i < explosionSources.size( ); i++ ) {
		alSourceStop( explosionSources[ i ] );
		alDeleteSources( 1, &explosionSources[ i ] );
	}

	alDeleteBuffers( 1, &shotSound );
	alDeleteBuffers( 1, &deathSound );
	alDeleteBuffers( 1, &damageSound );
	alDeleteBuffers( 1, &explosionSound );
	alDeleteBuffers( 1, &itemSound );

	alutExit( );
}

void GameViewSound::uninitActors( ) {
	for( std::map< std::string, ALuint >::iterator iter = actorShotSources.begin( ); iter != actorShotSources.end( ); ++iter ) {
		alSourceStop( iter->second );
		alDeleteSources( 1, &iter->second );
	}
	actorShotSources.clear( );

	for( std::map< std::string, ALuint >::iterator iter = actorDeathSources.begin( ); iter != actorDeathSources.end( ); ++iter ) {
		alSourceStop( iter->second );
		alDeleteSources( 1, &iter->second );
	}
	actorDeathSources.clear( );

	for( std::map< std::string, ALuint >::iterator iter = actorDamageSources.begin( ); iter != actorDamageSources.end( ); ++iter ) {
		alSourceStop( iter->second );
		alDeleteSources( 1, &iter->second );
	}
	actorDamageSources.clear( );

	for( std::map< std::string, ALuint >::iterator iter = actorItemSources.begin( ); iter != actorItemSources.end( ); ++iter ) {
		alSourceStop( iter->second );
		alDeleteSources( 1, &iter->second );
	}
	actorItemSources.clear( );
}

void GameViewSound::setCamera( const ActorModel * _camera ) {
	camera = _camera;
}

void GameViewSound::setGame( const GameModel * _game ) {
	game = _game;
}

void GameViewSound::update( float time ) {
	//music.update( );

	if( camera ) {
		ALfloat listenerposition [] = { camera->getPosition( )[ 0 ], 64.0f, camera->getPosition( )[ 2 ] };
		ALfloat listenervelocity [] = { camera->getVelocity( )[ 0 ], camera->getVelocity( )[ 1 ], camera->getVelocity( )[ 2 ] };
		ALfloat listenerorientation [] = { camera->getPosition( )[ 0 ], 0.0f, camera->getPosition( )[ 2 ], 0.0f, 0.0f, -1.0f };

		alListenerfv( AL_POSITION, listenerposition );
		alListenerfv( AL_VELOCITY, listenervelocity );
		alListenerfv( AL_ORIENTATION, listenerorientation );
	} else {
		ALfloat listenerposition [] = { 0.0f, 64.0f, 0.0f };
		ALfloat listenervelocity [] = { 0.0f, 0.0f, 0.0f };
		ALfloat listenerorientation [] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f };

		alListenerfv( AL_POSITION, listenerposition );
		alListenerfv( AL_VELOCITY, listenervelocity );
		alListenerfv( AL_ORIENTATION, listenerorientation );
	}
}

void GameViewSound::itemPickedUp( const ActorModel & actor, const Item * item, float starttime ) {
	ALfloat sourceposition [] = { actor.getPosition( )[ 0 ], actor.getPosition( )[ 1 ], actor.getPosition( )[ 2 ] };
	ALfloat sourcevelocity [] = { actor.getVelocity( )[ 0 ], actor.getVelocity( )[ 1 ], actor.getVelocity( )[ 2 ] };

	alSourcei( actorItemSources[ actor.getName( ) ], AL_BUFFER, itemSound );
	alSourcefv( actorItemSources[ actor.getName( ) ], AL_POSITION, sourceposition );
	alSourcefv( actorItemSources[ actor.getName( ) ], AL_VELOCITY, sourcevelocity );

	alSourcePlay( actorItemSources[ actor.getName( ) ] );
}

void GameViewSound::addActor( const ActorModel & actor ) {
	alGenSources( 1, &actorShotSources[ actor.getName( ) ] );
	alSourcef( actorShotSources[ actor.getName( ) ], AL_PITCH, 1.0f );
	alSourcef( actorShotSources[ actor.getName( ) ], AL_GAIN, 1.0f );
	alSourcei( actorShotSources[ actor.getName( ) ], AL_LOOPING, false );

	alGenSources( 1, &actorDeathSources[ actor.getName( ) ] );
	alSourcef( actorDeathSources[ actor.getName( ) ], AL_PITCH, 1.0f );
	alSourcef( actorDeathSources[ actor.getName( ) ], AL_GAIN, 1.0f );
	alSourcei( actorDeathSources[ actor.getName( ) ], AL_LOOPING, false );

	alGenSources( 1, &actorDamageSources[ actor.getName( ) ] );
	alSourcef( actorDamageSources[ actor.getName( ) ], AL_PITCH, 1.0f );
	alSourcef( actorDamageSources[ actor.getName( ) ], AL_GAIN, 1.0f );
	alSourcei( actorDamageSources[ actor.getName( ) ], AL_LOOPING, false );

	alGenSources( 1, &actorItemSources[ actor.getName( ) ] );
	alSourcef( actorItemSources[ actor.getName( ) ], AL_PITCH, 1.0f );
	alSourcef( actorItemSources[ actor.getName( ) ], AL_GAIN, 1.0f );
	alSourcei( actorItemSources[ actor.getName( ) ], AL_LOOPING, false );
}

void GameViewSound::removeActor( const ActorModel & actor ) {
	if( camera == &actor ) {
		camera = 0;
	}

	alSourceStop( actorShotSources[ actor.getName( ) ] );
	alDeleteSources( 1, &actorShotSources[ actor.getName( ) ] );
	actorShotSources.erase( actor.getName( ) );

	alSourceStop( actorDeathSources[ actor.getName( ) ] );
	alDeleteSources( 1, &actorDeathSources[ actor.getName( ) ] );
	actorDeathSources.erase( actor.getName( ) );

	alSourceStop( actorDamageSources[ actor.getName( ) ] );
	alDeleteSources( 1, &actorDamageSources[ actor.getName( ) ] );
	actorDamageSources.erase( actor.getName( ) );

	alSourceStop( actorItemSources[ actor.getName( ) ] );
	alDeleteSources( 1, &actorItemSources[ actor.getName( ) ] );
	actorItemSources.erase( actor.getName( ) );
}

void GameViewSound::actorSpawned( const ActorModel & actor, float starttime ) {
}

void GameViewSound::actorDamaged( const ActorModel * attacker, const ActorModel & attackee, float damage, float time ) {
	/*ALfloat sourceposition [] = { attackee.getPosition( )[ 0 ], attackee.getPosition( )[ 1 ], attackee.getPosition( )[ 2 ] };
	ALfloat sourcevelocity [] = { attackee.getVelocity( )[ 0 ], attackee.getVelocity( )[ 1 ], attackee.getVelocity( )[ 2 ] };

	alSourcei( actorDamageSources[ attackee.getName( ) ], AL_BUFFER, damageSound );
	alSourcefv( actorDamageSources[ attackee.getName( ) ], AL_POSITION, sourceposition );
	alSourcefv( actorDamageSources[ attackee.getName( ) ], AL_VELOCITY, sourcevelocity );

	alSourcePlay( actorDamageSources[ attackee.getName( ) ] );*/
}

void GameViewSound::actorKilled( const ActorModel * killer, const ActorModel & killee, float starttime ) {
	ALfloat sourceposition [] = { killee.getPosition( )[ 0 ], killee.getPosition( )[ 1 ], killee.getPosition( )[ 2 ] };
	ALfloat sourcevelocity [] = { killee.getVelocity( )[ 0 ], killee.getVelocity( )[ 1 ], killee.getVelocity( )[ 2 ] };

	alSourcei( actorDeathSources[ killee.getName( ) ], AL_BUFFER, deathSound );
	alSourcefv( actorDeathSources[ killee.getName( ) ], AL_POSITION, sourceposition );
	alSourcefv( actorDeathSources[ killee.getName( ) ], AL_VELOCITY, sourcevelocity );

	alSourcePlay( actorDeathSources[ killee.getName( ) ] );
}

void GameViewSound::projectileCreation( const ActorModel & actor, const vec3f & position, float starttime ) {
	ALfloat sourceposition [] = { actor.getPosition( )[ 0 ], actor.getPosition( )[ 1 ], actor.getPosition( )[ 2 ] };
	ALfloat sourcevelocity [] = { actor.getVelocity( )[ 0 ], actor.getVelocity( )[ 1 ], actor.getVelocity( )[ 2 ] };

	alSourcei( actorShotSources[ actor.getName( ) ], AL_BUFFER, shotSound );
	alSourcefv( actorShotSources[ actor.getName( ) ], AL_POSITION, sourceposition );
	alSourcefv( actorShotSources[ actor.getName( ) ], AL_VELOCITY, sourcevelocity );

	alSourcePlay( actorShotSources[ actor.getName( ) ] );
}

void GameViewSound::projectileCollision( const vec3f & position, const vec3f & normal, float starttime ) {
}

void GameViewSound::projectileExplosion( const vec3f & position, float starttime ) {
	ALfloat sourceposition [] = { position[ 0 ], position[ 1 ], position[ 2 ] };
	ALfloat sourcevelocity [] = { 0.0f, 0.0f, 0.0f };

	alSourcei( explosionSources[ explosionIndex ], AL_BUFFER, explosionSound );
	alSourcefv( explosionSources[ explosionIndex ], AL_POSITION, sourceposition );
	alSourcefv( explosionSources[ explosionIndex ], AL_VELOCITY, sourcevelocity );

	alSourcePlay( explosionSources[ explosionIndex ] );

	explosionIndex = ( explosionIndex + 1 ) % explosionSources.size( );
}
