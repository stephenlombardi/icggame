/*****************************
Author: Steve Lombardi
Description: Third person
actor controller
*****************************/
#ifndef _ActorControllerKeyboardTP_h
#define _ActorControllerKeyboardTP_h

#include "ActorControllerKeyboard.h"
#include "ActorModel.h"
#include "GameView.h"

#include <slib/Matrix.hpp>

class ActorControllerKeyboardTP : public ActorControllerKeyboard {
public:
	ActorControllerKeyboardTP( const GameView & _view );

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
	void calcMousePoint( );

	const ActorModel * model;
	const GameView & view;
	bool keyboard[ 256 ];
	float mousex, mousey;
	float worldx, worldy;
	bool mousebuttons[ 3 ];
};

#endif