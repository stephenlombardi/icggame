#include "ActorModel.h"

//ActorModel::ActorModel( ) : currentWeapon( 0 ) { }
//ActorModel::ActorModel( const std::string & _name, float _mass, float _boundingRadius, float _speed ) : name( _name ), position( vec3f( 0.0f ) ), velocity( vec3f( 0.0f ) ), orientation( Vector3( 1.0f, 0.0f, 0.0f ) ), force( vec3f( 0.0f ) ), health( 0.0f ), mass( _mass ), boundingRadius( _boundingRadius ), speed( _speed ), state( "idle" ), stateStart( 0.0f ), currentWeapon( 0 ), homingPowerup( false ), attackDelayModifier( 1.0f ), speedModifier( 1.0f ), powerupLevel( 0 ) { }
ActorModel::ActorModel( const std::string & _name, float _mass, float _boundingRadius, float _speed ) : name( _name ), position( vec3f( 0.0f ) ), velocity( vec3f( 0.0f ) ), orientation( Vector3( 1.0f, 0.0f, 0.0f ) ), force( vec3f( 0.0f ) ), health( 0.0f ), mass( _mass ), boundingRadius( _boundingRadius ), speed( _speed ), deathtime( 0.0f ), currentWeapon( 0 ), homingPowerup( false ), attackDelayModifier( 1.0f ), speedModifier( 1.0f ), powerupLevel( 0 ) { }

const std::string & ActorModel::getName( ) const { return name; }

// game stuff
float & ActorModel::getHealth( ) { return health; }
float ActorModel::getHealth( ) const { return health; }

bool & ActorModel::hasHomingPowerup( ) { return homingPowerup; }
bool ActorModel::hasHomingPowerup( ) const { return homingPowerup; }

float & ActorModel::getAttackDelayModifier( ) { return attackDelayModifier; }
float ActorModel::getAttackDelayModifier( ) const { return attackDelayModifier; }

float & ActorModel::getSpeedModifier( ) { return speedModifier; }
float ActorModel::getSpeedModifier( ) const { return speedModifier; }

int & ActorModel::getPowerupLevel( ) { return powerupLevel; }
int ActorModel::getPowerupLevel( ) const { return powerupLevel; }

Weapon & ActorModel::getCurrentWeapon( ) { assert( currentWeapon ); return *currentWeapon; }
const Weapon & ActorModel::getCurrentWeapon( ) const { assert( currentWeapon ); return *currentWeapon; }

bool ActorModel::addWeapon( Weapon weapon ) { 
	bool found = false;
	for( std::list< Weapon >::iterator iter = weapons.begin( ); iter != weapons.end( ); ++iter ) {
		if( weapon == *iter ) {
			found = true;
			break;
		}
	}

	if( found ) {
		return false;
	} else {
		weapons.push_back( weapon );
		if( !currentWeapon ) {
			currentWeapon = &weapons.back( );
		}
		return true;
	}
}

void ActorModel::switchWeapon( int weapon ) {
	std::list< Weapon >::iterator iter = weapons.begin( );
	for( int i = 0; i < weapon && iter != weapons.end( ); ++iter, ++i );
	if( iter != weapons.end( ) ) {
		currentWeapon = &*iter;
	}
}

const std::list< Weapon > & ActorModel::getWeapons( ) const {
	return weapons;
}

void ActorModel::removeWeapons( ) {
	weapons.clear( );
	currentWeapon = 0;
}

// something
//std::string & ActorModel::getState( ) { return state; }
//const std::string & ActorModel::getState( ) const { return state; }

//float & ActorModel::getStateStart( ) { return stateStart; }
//float ActorModel::getStateStart( ) const { return stateStart; }

float & ActorModel::getDeathTime( ) { return deathtime; }
float ActorModel::getDeathTime( ) const { return deathtime; }

// physics stuff
vec3f & ActorModel::getPosition( ) { return position; }
const vec3f & ActorModel::getPosition( ) const { return position; }

vec3f & ActorModel::getVelocity( ) { return velocity; }
const vec3f & ActorModel::getVelocity( ) const { return velocity; }

vec3f & ActorModel::getForce( ) { return force; }
const vec3f & ActorModel::getForce( ) const { return force; }

vec3f & ActorModel::getOrientation( ) { return orientation; }
const vec3f & ActorModel::getOrientation( ) const { return orientation; }

float & ActorModel::getMass( ) { return mass; }
float ActorModel::getMass( ) const { return mass; }

float & ActorModel::getBoundingRadius( ) { return boundingRadius; }
float ActorModel::getBoundingRadius( ) const { return boundingRadius; }

float & ActorModel::getSpeed( ) { return speed; }
float ActorModel::getSpeed( ) const { return speed; }
