#ifndef _ActorControllerNull_h
#define _ActorControllerNull_h

#include <slib/Matrix.hpp>

class ActorControllerNull : public ActorController {
public:
	vec3f getMoveDirection( ) const {
		return Vector3( 0.0f, 0.0f, 0.0f );
	}

	vec3f getLookDirection( ) const {
		return Vector3( 1.0f, 0.0f, 0.0f );
	}

	int getWeaponSwitch( ) const {
		return -1;
	}

	bool isAttacking( ) const {
		return false;
	}
};

#endif