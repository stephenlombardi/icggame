/*****************************
Author: Steve Lombardi
Description: First person
actor controller
*****************************/

#ifndef _ActorControllerKeyboardFP_h
#define _ActorControllerKeyboardFP_h

#include "ActorControllerKeyboard.h"
#include "ActorModel.h"
#include "GameView.h"

#include <slib/Matrix.hpp>

class ActorControllerKeyboardFP : public ActorControllerKeyboard {
public:
	ActorControllerKeyboardFP( const GameView & _view );

	vec3f getMoveDirection( ) const;
	vec3f getLookDirection( ) const;
	int getWeaponSwitch( ) const;
	bool isAttacking( ) const;

	bool & getKey( int i );
	float getMouseX( ) const;
	float getMouseY( ) const;
	void setMouse( float _mousex, float _mousey );

	bool & getMouseButton( int i );

	void setModel( const ActorModel * _model );
private:
	const ActorModel * model;
	const GameView & view;
	bool keyboard[ 256 ];
	float mousex, mousey;
	float angle;
	bool mousebuttons[ 3 ];
};

#endif 