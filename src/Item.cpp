#include "Item.h"
#include "GameModel.h"

ItemWeapon::ItemWeapon( const vec3f & _position, const Weapon & _weapon ) : position( _position ), weapon( _weapon ) { }
vec3f & ItemWeapon::getPosition( ) {
	return position;
}
const vec3f & ItemWeapon::getPosition( ) const {
	return position;
}
bool ItemWeapon::activate( GameModel * game, ActorModel * model ) {
	return model->addWeapon( weapon );
}

const Weapon & ItemWeapon::getWeapon( ) const {
	return weapon;
}

/*ItemHealth::ItemHealth( const vec3f & _position, float _health ) : position( _position ), health( _health ) { }
vec3f & ItemHealth::getPosition( ) {
	return position;
}
const vec3f & ItemHealth::getPosition( ) const {
	return position;
}
bool ItemHealth::activate( GameModel * game, ActorModel * model ) {
	model->getHealth( ) += health;
	return true;
}*/

ItemPowerup::ItemPowerup( const vec3f & _position, const std::string & _type ) : position( _position ), type( _type ) { }
vec3f & ItemPowerup::getPosition( ) {
	return position;
}
const vec3f & ItemPowerup::getPosition( ) const {
	return position;
}
const std::string & ItemPowerup::getType( ) const {
	return type;
}
bool ItemPowerup::activate( GameModel * game, ActorModel * model ) {
	// activate powerup
	//model->getPowerupLevel( )++;
	if( type == "homing" ) {
		model->hasHomingPowerup( ) = true;
	} else if( type == "attackspeed" ) {
		model->getAttackDelayModifier( ) = 0.5f;
	} else if( type == "speed" ) {
		model->getSpeedModifier( ) = 1.5f;
	} else if( type == "health" ) {
		model->getHealth( ) += 20.0f;
	}
	return true;
}
