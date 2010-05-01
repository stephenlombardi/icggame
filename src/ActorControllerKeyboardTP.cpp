#include "ActorControllerKeyboardTP.h"

ActorControllerKeyboardTP::ActorControllerKeyboardTP( const GameView & _view ) : view( _view ) {
	for( int i = 0; i < 256; i++ ) {
		keyboard[ i ] = false;
	}

	for( int i = 0; i < 3; i++ ) {
		mousebuttons[ i ] = false;
	}
}

vec3f ActorControllerKeyboardTP::getMoveDirection( ) const {
	vec3f dir( 0.0f );
	if( keyboard[ 'w' ] ) {
		dir += Vector3( 0.0f, 0.0f, -1.0f );
	}
	if( keyboard[ 's' ] ) {
		dir += Vector3( 0.0f, 0.0f, 1.0f );
	}
	if( keyboard[ 'a' ] ) {
		dir += Vector3( -1.0f, 0.0f, 0.0f );
	}
	if( keyboard[ 'd' ] ) {
		dir += Vector3( 1.0f, 0.0f, 0.0f );
	}
	float norm = Norm( dir );
	return norm > 0.0f ? Normalize( dir ) : dir;
}

vec3f ActorControllerKeyboardTP::getLookDirection( ) const { return Normalize( Vector3( worldx - model->getPosition( )[ 0 ], 0.0f, worldy - model->getPosition( )[ 2 ] ) ); }
int ActorControllerKeyboardTP::getWeaponSwitch( ) const {
	return keyboard[ '1' ] ? 0 :
		keyboard[ '2' ] ? 1 :
		keyboard[ '3' ] ? 2 :
		keyboard[ '4' ] ? 3 :
		keyboard[ '5' ] ? 4 :
		keyboard[ '6' ] ? 5 :
		keyboard[ '7' ] ? 6 :
		-1;
}
bool ActorControllerKeyboardTP::isAttacking( ) const { return mousebuttons[ 0 ]; }

bool & ActorControllerKeyboardTP::getKey( int i ) { return keyboard[ i ]; }
float ActorControllerKeyboardTP::getMouseX( ) const { return mousex; }
float ActorControllerKeyboardTP::getMouseY( ) const { return mousey; }
void ActorControllerKeyboardTP::setMouse( float _mousex, float _mousey ) { mousex = _mousex; mousey = _mousey; calcMousePoint( ); }
void ActorControllerKeyboardTP::calcMousePoint( ) {
	float viewm[ 16 ], projectionm[ 16 ];
	view.getViewMatrix( viewm );
	view.getProjectionMatrix( projectionm );

	Matrix< 4, 4, float > viewprojection = RowMajor< 4, 4, float >( projectionm ) * RowMajor< 4, 4, float >( viewm );
	Matrix< 4, 1, float > centerpoint = viewprojection * Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
	Matrix< 4, 1, float > mouse = Vector4( mousex, mousey, centerpoint[ 2 ] / centerpoint[ 3 ], 1.0f );
	Matrix< 4, 1, float > unprojected = Solve( viewprojection, mouse );
	unprojected *= ( 1.0f / unprojected[ 3 ] );

	worldx = unprojected[ 0 ];
	worldy = unprojected[ 2 ];
}

bool & ActorControllerKeyboardTP::getMouseButton( int i ) { return mousebuttons[ i ]; }

void ActorControllerKeyboardTP::setModel( const ActorModel * _model ) { model = _model; }
