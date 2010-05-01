#ifndef _ActorController_h
#define _ActorController_h

#include <slib/Matrix.hpp>

using namespace slib;

class ActorController {
public:
	virtual vec3f getMoveDirection( ) const = 0;
	virtual vec3f getLookDirection( ) const = 0;
	virtual int getWeaponSwitch( ) const = 0;
	virtual bool isAttacking( ) const = 0;
};

#endif