/*****************************
Author: Steve Lombardi
Description: Actor class
*****************************/

#ifndef _ActorModel_h
#define _ActorModel_h

#include <cassert>
#include <string>
#include <list>

#include <slib/Matrix.hpp>

#include "Weapon.h"

using namespace slib;

class ActorModel {
public:
	//ActorModel( );
	ActorModel( const std::string & _name, float _mass, float _boundingRadius, float _speed );

	const std::string & getName( ) const;

	// game stuff
	float & getHealth( );
	float getHealth( ) const;

	bool & hasHomingPowerup( );
	bool hasHomingPowerup( ) const;

	float & getAttackDelayModifier( );
	float getAttackDelayModifier( ) const;

	float & getSpeedModifier( );
	float getSpeedModifier( ) const;

	int & getPowerupLevel( );
	int getPowerupLevel( ) const;
	
	Weapon & getCurrentWeapon( );
	const Weapon & getCurrentWeapon( ) const;
	bool addWeapon( Weapon weapon );
	void switchWeapon( int weapon );
	const std::list< Weapon > & getWeapons( ) const;
	void removeWeapons( );

	// something
	//std::string & getState( );
	//const std::string & getState( ) const;

	//float & getStateStart( );
	//float getStateStart( ) const;
	float & getDeathTime( );
	float getDeathTime( ) const;

	// physics stuff
	vec3f & getPosition( );
	const vec3f & getPosition( ) const;

	vec3f & getVelocity( );
	const vec3f & getVelocity( ) const;

	vec3f & getForce( );
	const vec3f & getForce( ) const;

	vec3f & getOrientation( );
	const vec3f & getOrientation( ) const;

	float & getMass( );
	float getMass( ) const;

	float & getBoundingRadius( );
	float getBoundingRadius( ) const;

	float & getSpeed( );
	float getSpeed( ) const;
private:
	std::string name;

	float health;

	bool homingPowerup;
	float attackDelayModifier;
	float speedModifier;
	int powerupLevel;

	std::list< Weapon > weapons;
	Weapon * currentWeapon;
	
	// switching weapon, attacking, idle, dying, etc.,
	//std::string state;
	//float stateStart;
	float deathtime;

	vec3f position, velocity, force, orientation;
	float mass, boundingRadius, speed;
};

#endif