/*****************************
Author: Steve Lombardi
Description: Projectile class
*****************************/

#ifndef _Projectile_h
#define _Projectile_h

#include <slib/Matrix.hpp>
#include <slib/PlanarVolume.hpp>

#include "Equation.h"
#include "TexturedPlane.h"

class ActorModel;
class GameModel;

using namespace slib;

class Projectile {
public:
	Projectile( float _mass, float _boundingRadius );
	virtual ~Projectile( );

	virtual Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;

	const ActorModel * getOwner( ) const;
	void setOwner( const ActorModel * _owner );

	virtual bool isDead( ) const;

	// returns true if projectile should be ignored for subsequent collision tests
	virtual bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	virtual bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	virtual bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );

	virtual void update( GameModel * game, float time );

	// physics
	vec3f & getPosition( );
	const vec3f & getPosition( ) const;

	vec3f & getVelocity( );
	const vec3f & getVelocity( ) const;

	vec3f & getForce( );
	const vec3f & getForce( ) const;

	float getMass( ) const;

	float getBoundingRadius( ) const;
protected:
	void damageActor( GameModel * game, ActorModel & actor, float damage, float time ) const;
	void checkActorHealth( GameModel * game, ActorModel & actor, float time ) const;
	void handleExplosion( GameModel * game, const Equation * forceEq, const Equation * damageEq, float time );

private:
	const ActorModel * owner;

	vec3f position, velocity, force;
	float mass, boundingRadius;
};

class ProjectileMelee : public Projectile {
public:
	ProjectileMelee( float _mass, float _boundingRadius, float _damage, float _alivetime );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
private:
	float damage;
	float starttime, alivetime;
	bool dead;
};

class ProjectileBullet : public Projectile {
public:
	ProjectileBullet( float _mass, float _boundingRadius, float _damage );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
private:
	float damage;
	bool dead;
};

class ProjectileBounceLaser : public Projectile {
public:
	ProjectileBounceLaser( float _mass, float _boundingRadius, float _damage );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
	float & getStartTime( );
private:
	float damage;
	float starttime;
	bool dead;
	int bounces;
};

class ProjectileRocket : public Projectile {
public:
	ProjectileRocket( float _mass, float _boundingRadius, float _damage, const Equation * _forceEq, const Equation * _damageEq );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
private:
	float damage;
	const Equation * forceEq, * damageEq;
	bool dead;
};

class ProjectileGrenade : public Projectile {
public:
	ProjectileGrenade( float _mass, float _boundingRadius, float _damage, float _alivetime, const Equation * _forceEq, const Equation * _damageEq );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
private:
	float damage;
	bool dead;
	float starttime, alivetime;
	const Equation * forceEq, * damageEq;
};

class ProjectileFlame : public Projectile {
public:
	ProjectileFlame( float _mass, float _boundingRadius, float _damage, float _alivetime );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );

	float getStartTime( ) const;
	float & getStartTime( );
	float getAliveTime( ) const;
private:
	float damage;
	bool dead;
	float starttime, alivetime;
};

class ProjectileBFG : public Projectile {
public:
	ProjectileBFG( float _mass, float _boundingRadius, float _damage, int _spawnprojs );
	Projectile * clone( const vec3f & _position, const ActorModel * _owner, float time ) const;
	bool isDead( ) const;
	bool handleActorCollision( GameModel * game, ActorModel & actor, float t, float time );
	bool handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time );
	bool handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time );
	void update( GameModel * game, float time );
private:
	void BFGExplode( GameModel * game, const vec3f & normal, float time );
	float damage;
	bool dead;
	int spawnprojs;
};



#endif