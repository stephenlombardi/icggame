/*****************************
Author: Steve Lombardi
Description: controls all audio
processing of the game
*****************************/

#ifndef _GameViewSound_h
#define _GameViewSound_h

#include <map>

#include <AL/alut.h>
#include <slib/OggStream.h>

#include "GameModel.h"
#include "GameObserver.h"

class GameViewSound : public GameObserver {
public:
	void init( );
	//void initActors( );
	void uninitActors( );
	void uninit( );
	void setCamera( const ActorModel * _camera );
	void setGame( const GameModel * _game );
	void update( float time );

	void itemPickedUp( const ActorModel & actor, const Item * item, float starttime );
	void addActor( const ActorModel & actor );
	void removeActor( const ActorModel & actor );
	void actorSpawned( const ActorModel & actor, float starttime );
	void actorDamaged( const ActorModel * attacker, const ActorModel & attackee, float damage, float time );
	void actorKilled( const ActorModel * killer, const ActorModel & killee, float starttime );
	void projectileCreation( const ActorModel & actor, const vec3f & position, float starttime );
	void projectileCollision( const vec3f & position, const vec3f & normal, float starttime );
	void projectileExplosion( const vec3f & position, float starttime );
private:
	OggStream music;

	ALuint shotSound, deathSound, damageSound, explosionSound, itemSound;

	std::map< std::string, ALuint > actorShotSources;
	std::map< std::string, ALuint > actorDeathSources;
	std::map< std::string, ALuint > actorDamageSources;
	std::map< std::string, ALuint > actorItemSources;

	std::vector< ALuint > explosionSources;
	int explosionIndex;

	const ActorModel * camera;
	const GameModel * game;
};

#endif