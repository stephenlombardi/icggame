#ifndef _Item_h
#define _Item_h

#include <slib/Matrix.hpp>

#include "ActorModel.h"

class GameModel;

class Item {
public:
	virtual vec3f & getPosition( ) = 0;
	virtual const vec3f & getPosition( ) const = 0;

	// returns true if the item is able to be picked up
	virtual bool activate( GameModel * game, ActorModel * model ) = 0;
};

class ItemWeapon : public Item {
public:
	ItemWeapon( const vec3f & _position, const Weapon & _weapon );
	vec3f & getPosition( );
	const vec3f & getPosition( ) const;
	bool activate( GameModel * game, ActorModel * model );
	const Weapon & getWeapon( ) const;
private:
	vec3f position;
	Weapon weapon;
};

/*class ItemHealth : public Item {
public:
	ItemHealth( const vec3f & _position, float _health );
	vec3f & getPosition( );
	const vec3f & getPosition( ) const;
	bool activate( GameModel * game, ActorModel * model );
private:
	vec3f position;
	float health;
};*/

class ItemPowerup : public Item {
public:
	ItemPowerup( const vec3f & _position, const std::string & _type );
	vec3f & getPosition( );
	const vec3f & getPosition( ) const;
	const std::string & getType( ) const;
	bool activate( GameModel * game, ActorModel * model );
private:
	vec3f position;
	std::string type;
};


#endif