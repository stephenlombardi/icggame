#ifndef _GameSetup_h
#define _GameSetup_h

#include "GameModel.h"
#include "GameView.h"
#include "GameViewSound.h"
#include "Weapon.h"
#include "ActorModel.h"
#include "ActorController.h"
#include "ActorControllerKeyboard.h"
#include "ActorControllerKeyboardFP.h"
#include "ActorControllerKeyboardTP.h"
#include "ActorControllerAI.h"

#include <slib/ParseMap.h>
#include <slib/PathGraph.h>

#include <map>
#include <string>

void initGlobals( std::map< std::string, float > & globalVariables );

void resetGameLoadMapAddActors( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & filename );

void resetGame( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound );
void setGameVars( GameModel & game, std::map< std::string, float > & globalVariables );

void loadMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, std::map< std::string, float > & globalVariables, const std::string & filename );
PlanarVolume< 3, float, TexturedPlane< 3, float > > mapBrushToPlanarVolume( const Map::Brush & mapbrush );
void addWorldspawn( GameModel & game, GameView & view, Map::Entity & entity );
void constructPathGraph( GameModel & game, PathGraph & pathgraph );

//void addPlayer( GameModel & game, ActorControllerKeyboard * keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & name );
//void addAI( GameModel & game, ActorControllerAI * aiController, std::map< std::string, float > & globalVariables, float gametime, const std::string & name );
template< class ControllerT >
void addActor( GameModel & game, ControllerT * controller, std::map< std::string, float > & globalVariables, float gametime, const std::string & name ) {
	//game.addActor( Actor( keyboardController, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) ) );
	game.addActor( controller, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) );
	game.spawnActor( game.getActors( ).back( ), gametime );
	controller->setModel( &game.getActors( ).back( ) );
	addWeapons( game.getActors( ).back( ), globalVariables );
	//return &game.getActors( ).back( );
}

void addWeapons( ActorModel & actor, std::map< std::string, float > & globalVariables );
//void removeWeapons( ActorModel & actor );
void resetActorVars( GameModel & game, std::map< std::string, float > & globalVariables );

Weapon createKnife( std::map< std::string, float > & globalVariables );
Weapon createMachinegun( std::map< std::string, float > & globalVariables );
Weapon createRocketlauncher( std::map< std::string, float > & globalVariables );
Weapon createShotgun( std::map< std::string, float > & globalVariables );


///////////////////// extra /////////////////////////////////
void setFirstPerson( GameModel & game, GameView & view, ActorControllerKeyboard * & keyboardController, bool firstperson );
void setGlobal( GameModel & game, std::map< std::string, float > & globalVariables, const std::string & varname, float value );
std::string getGlobal( std::map< std::string, float > & globalVariables, const std::string & varname );
void setCamera( GameView & view, GameViewSound & sound, const ActorModel * actor );
void setMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & filename );
void addPlayer( GameModel & game, ActorControllerKeyboard * keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & name );
void addAI( GameModel & game, PathGraph & pathgraph, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & name );
void removeActor( GameModel & game, std::map< std::string, ActorControllerAI * > & aiControllers, const std::string & actorname );


#endif