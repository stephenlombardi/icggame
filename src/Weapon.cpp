#include "Weapon.h"
//#include "Actor.h"
#include "ActorModel.h"

Weapon::Weapon( ) : projmodel( 0 ) { }

Weapon::Weapon( const Weapon & rhs ) : projmodel( rhs.projmodel->clone( vec3f( 0.0f ), 0, 0.0f ) ), attackdelay( rhs.attackdelay ), muzzlespeed( rhs.muzzlespeed ), spread( rhs.spread ), projcount( rhs.projcount ), lastattack( 0.0f ) { }

//Weapon::Weapon( float _damage, float _attackdelay, float _muzzlespeed, float _spread, float _projmass, float _boundingRadius, bool _explosion, Equation * _forceEq, Equation * _damageEq ) : damage( _damage ), attackdelay( _attackdelay ), muzzlespeed( _muzzlespeed ), spread( _spread ), projmass( _projmass ), boundingRadius( _boundingRadius ), explosion( _explosion ), forceEq( _forceEq ), damageEq( _damageEq ), lastattack( 0.0f ) { }
Weapon::Weapon( const Projectile * _projmodel, float _attackdelay, float _muzzlespeed, float _spread, int _projcount ) : projmodel( _projmodel ), attackdelay( _attackdelay ), muzzlespeed( _muzzlespeed ), spread( _spread ), projcount( _projcount ), lastattack( 0.0f ) { }

Weapon & Weapon::operator=( const Weapon & rhs ) {
	projmodel = rhs.projmodel->clone( vec3f( 0.0f ), 0, 0.0f );
	attackdelay = rhs.attackdelay;
	muzzlespeed = rhs.muzzlespeed;
	spread = rhs.spread;
	projcount = rhs.projcount;
	lastattack = 0.0f;
	return *this;
}

Weapon::~Weapon( ) {
	delete projmodel;
}

bool Weapon::isReady( float time, float delaymodifier ) const {
	return time - lastattack > attackdelay * delaymodifier;
}

float Weapon::getMuzzleSpeed( ) const {
	return muzzlespeed;
}

float Weapon::getSpread( ) const {
	return spread;
}

std::list< Projectile * > Weapon::createProjectile( const vec3f & position, float time, const ActorModel * owner ) {
	lastattack = time;

	std::list< Projectile * > result;
	for( int i = 0; i < projcount; i++ ) {
		result.push_back( projmodel->clone( position, owner, time ) );
	}
	return result;
}

const Projectile * Weapon::getModel( ) const {
	return projmodel;
}

bool Weapon::operator==( const Weapon & rhs ) const {
	return std::string( typeid( *projmodel ).name( ) ) == std::string( typeid( *rhs.projmodel ).name( ) ) && attackdelay == rhs.attackdelay && muzzlespeed == rhs.muzzlespeed && spread == rhs.spread;
}
