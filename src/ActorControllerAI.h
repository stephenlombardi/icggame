#ifndef _ActorControllerAI_h
#define _ActorControllerAI_h

#include "ActorController.h"
#include "ActorModel.h"
#include "GameModel.h"
#include <slib/PathGraph.h>

#include <slib/Matrix.hpp>

class ActorControllerAI : public ActorController {
public:
	ActorControllerAI( const GameModel * _game, const PathGraph & _pathgraph );

	vec3f getMoveDirection( ) const;
	vec3f getLookDirection( ) const;
	int getWeaponSwitch( ) const;
	bool isAttacking( ) const;
	void setModel( const ActorModel * _actormodel );
	float calculateEnergy( int vertex ) const;
	vec3f calculateThreat( ) const;
	int findClosestVertex( const vec3f & position ) const;
	void update( float time );
private:
	vec3f greatestthreat;
	int nextvertex;
	bool attacking;
	int weaponselection;

	const ActorModel * actormodel;
	const GameModel * game;
	const PathGraph & pathgraph;
};

#endif