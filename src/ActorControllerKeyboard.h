#ifndef _ActorControllerKeyboard_h
#define _ActorControllerKeyboard_h

#include "ActorController.h"
#include "ActorModel.h"

#include <slib/Matrix.hpp>

class ActorControllerKeyboard : public ActorController {
public:
	virtual vec3f getMoveDirection( ) const = 0;
	virtual vec3f getLookDirection( ) const = 0;
	virtual int getWeaponSwitch( ) const = 0;
	virtual bool isAttacking( ) const = 0;

	virtual bool & getKey( int i ) = 0;
	virtual bool & getMouseButton( int i ) = 0;
	virtual void setMouse( float x, float y ) = 0;

	virtual void setModel( const ActorModel * _actormodel ) = 0;
};

#endif