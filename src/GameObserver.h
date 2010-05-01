#ifndef _GameObserver_h
#define _GameObserver_h

#include "Projectile.h"
#include "ActorModel.h"
#include "Item.h"

#include <slib/Matrix.hpp>

//typedef Matrix< 3, 1, float > vec3f;

class GameObserver {
public:
	virtual void itemPickedUp( const ActorModel & actor, const Item * item, float starttime ) = 0;
	virtual void addActor( const ActorModel & actor ) = 0;
	virtual void removeActor( const ActorModel & actor ) = 0;
	virtual void actorSpawned( const ActorModel & actor, float starttime ) = 0;
	virtual void actorDamaged( const ActorModel * attacker, const ActorModel & attackee, float damage, float time ) = 0;
	virtual void actorKilled( const ActorModel * killer, const ActorModel & killee, float starttime ) = 0;
	virtual void projectileCreation( const ActorModel & actor, const vec3f & position, float starttime ) = 0;
	virtual void projectileCollision( const vec3f & position, const vec3f & normal, float starttime ) = 0;
	virtual void projectileExplosion( const vec3f & position, float starttime ) = 0;
private:
};

#endif