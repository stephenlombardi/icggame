#include "ActorControllerKeyboardFP.h"

ActorControllerKeyboardFP::ActorControllerKeyboardFP( const GameView & _view ) : view( _view ), mousex( 0.0f ), mousey( 0.0f ), angle( 0.0f ) {
	for( int i = 0; i < 256; i++ ) {
		keyboard[ i ] = false;
	}

	for( int i = 0; i < 3; i++ ) {
		mousebuttons[ i ] = false;
	}
}

vec3f ActorControllerKeyboardFP::getMoveDirection( ) const {
	vec3f dir( 0.0f );
	if( keyboard[ 'w' ] ) {
		dir += model->getOrientation( );
	}
	if( keyboard[ 's' ] ) {
		dir -= model->getOrientation( );
	}
	if( keyboard[ 'a' ] ) {
		dir += Vector3( model->getOrientation( )[ 2 ], 0.0f, -model->getOrientation( )[ 0 ] );
	}
	if( keyboard[ 'd' ] ) {
		dir -= Vector3( model->getOrientation( )[ 2 ], 0.0f, -model->getOrientation( )[ 0 ] );
	}
	float norm = Norm( dir );
	return norm > 0.0f ? Normalize( dir ) : dir;
}

vec3f ActorControllerKeyboardFP::getLookDirection( ) const { return Vector3( std::cos( angle * 3.0f ), 0.0f, std::sin( angle * 3.0f ) ); }
int ActorControllerKeyboardFP::getWeaponSwitch( ) const {
	return keyboard[ '1' ] ? 0 :
		keyboard[ '2' ] ? 1 :
		keyboard[ '3' ] ? 2 :
		keyboard[ '4' ] ? 3 :
		keyboard[ '5' ] ? 4 :
		keyboard[ '6' ] ? 5 :
		keyboard[ '7' ] ? 6 :
		-1;
}
bool ActorControllerKeyboardFP::isAttacking( ) const { return mousebuttons[ 0 ]; }

bool & ActorControllerKeyboardFP::getKey( int i ) { return keyboard[ i ]; }
float ActorControllerKeyboardFP::getMouseX( ) const { return mousex; }
float ActorControllerKeyboardFP::getMouseY( ) const { return mousey; }
void ActorControllerKeyboardFP::setMouse( float _mousex, float _mousey ) {
	mousex = _mousex;
	mousey = _mousey;
	angle += mousex;
}

bool & ActorControllerKeyboardFP::getMouseButton( int i ) { return mousebuttons[ i ]; }

void ActorControllerKeyboardFP::setModel( const ActorModel * _model ) { model = _model; }
