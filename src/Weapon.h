/*****************************
Author: Steve Lombardi
Description: Weapon information
*****************************/

#ifndef _Weapon_h
#define _Weapon_h

#include <iostream>
#include <string>

#include <slib/Matrix.hpp>

#include "Projectile.h"

class ActorModel;

using namespace slib;

class Weapon {
public:
	Weapon( );
	Weapon( const Weapon & rhs );
	// no need to delete the projmodel that is passed in, the destructor will do that
	Weapon( const Projectile * _projmodel, float _attackdelay, float _muzzlespeed, float _spread, int _projcount );
	Weapon & operator=( const Weapon & rhs );
	~Weapon( );
	
	bool isReady( float time, float delaymodifier ) const;
	float getMuzzleSpeed( ) const;
	float getSpread( ) const;
	
	std::list< Projectile * > createProjectile( const vec3f & position, float time, const ActorModel * owner );
	const Projectile * getModel( ) const;

	bool operator==( const Weapon & rhs ) const;
private:
	const Projectile * projmodel;
	float attackdelay;
	float lastattack;
	float muzzlespeed;
	float spread;
	int projcount;
};

#endif