/****************************************************
* Author: Steve Lombardi
* Date: February-April, 2010
* Description: Final Project for Interactive Computer
*    Graphics at Drexel University in OpenGL 3.2
****************************************************/

/****************************************************
Controls:
'w' moves up
's' moves down
'a' moves left
'd' moves right
mouse aims
left click attacks
space changes between first person and third person view
'`' toggles the console (and pauses the game)
****************************************************/

/****************************************************
Requires a DX10/OGL3 compatable graphics card
Dependencies: opengl, freeglut, openal, alut, libvorbis, libogg
To see the game in action:
http://www.youtube.com/watch?v=iaKQvQ57QFY
****************************************************/

// todo: new commands
//       add/remove light
//          named lights?
//       
// todo: consolidate items somehow, make them scriptable
// todo: make weapons scriptable, modular

// todo: finish deferred renderer
//       winding on cone is backwards, maybe create it with -1 height
//       variable sized (fov) spotlights
//       glow in alpha channel
// todo: console commands to enable/disable/set light parameters
// todo: triggers
// todo: doors
// todo: integrate scripting

// todo: change load-map command so that it only loads a map, doesn't add actors
// todo: remove uninitActorText from GameView, remove each actor instead
// tood: use boost::bind in gameview
// todo: use unique ids instead of names
// todo: change initialially added actors
// todo: refactor gameconsole code
//           map of functions to process each type
// todo: re-figure out how actors store weapons
//           don't make having a current weapon necessary
// todo: re-figure out how weapons work
// todo: handle 'dead state' where no map is loaded initially
// todo: refactor load map
//           map of functions to process each type
// todo: make load map return bool to indicate sucess
// todo: start a documentation doc
// todo: rewrite parsemap code
// todo: more error handling
// todo: get rid of mutable getActors method in gamemodel class
// todo: rethink how to do projectile without polymorphism
// todo: speed projectile rendering
// todo: make view stuff more robust (check to see if actorText entries exist before updating)
// todo: consolidate item spawns into one (powerup)
// todo: fix respawn effect
// todo: remove unnecssary include statements
// todo: fix up/organize console

// todo: add console commands:
//          disable ai updates
//          enable/disable rendering things

// todo: new map
// todo: invulnerability on spawn ?
// todo: flamethrower is weak
// todo: change the weapon spread so that it spreads in an angle

// todo: different sound effects for different weapons

// make width/height variable

// todo: optimize:
//           queue sounds in a buffer when GameObserver methods are called
//           only update hud text in update method, instead of GameObserver method
//           implement navmeshes
// todo: clean up projectile code
// todo: check for memory leaks
// todo: clean up code

#include <map>
#include <string>

#include <boost/function.hpp>

#include <AL/alut.h>

#include <GL/freeglut.h>
#include <GL/glext.h>
#include <slib/extensions.h>

#include <slib/Matrix.hpp>
#include <slib/smf.h>
#include <slib/shader.h>
#include <slib/drawableobject.h>
#include <slib/shapes.h>
#include <slib/matrixutils.h>
#include <slib/PlanarVolume.hpp>
#include <slib/ParseMap.h>
#include <slib/Menu.h>
#include <slib/MenuRendererOpenGL.h>

#include "ActorModel.h"
#include "ActorController.h"
#include "ActorControllerKeyboard.h"
#include "ActorControllerKeyboardFP.h"
#include "ActorControllerKeyboardTP.h"
#include "ActorControllerAI.h"
#include "ActorControllerNull.h"
#include "ActorModel.h"
#include "GameModel.h"
#include "GameView.h"
#include "GameViewSound.h"
#include "Weapon.h"
#include "Projectile.h"

#include "GameSetup.h"
#include "GameConsole.h"

using namespace slib;

// globals
ActorControllerKeyboard * keyboardController;
std::map< std::string, ActorControllerAI * > aiControllers;
GameModel game;
GameView view;
GameViewSound sound;
PathGraph pathgraph;
std::map< std::string, float > globalVariables;

// console
std::map< std::string, boost::function< std::string (const std::string &) > > consolefuncs;
std::list< std::string > executedcmds;
std::list< std::string >::iterator executedcmdsiter;


float gametime = 0.0f;
int mousex = 0, mousey = 0;
int width = 768, height = 768;
bool aienabled = false;

// menu
bool consoleup = false;
bool menuup = false;

MenuRendererOpenGL * menuRenderer;
Menu::Frame * console;
Menu::Textbox * consoleinput;
Menu::MultiTextbox * consoleoutput;
Menu::Frame * menu;
Menu::Button * resumeButton;
Menu::Button * exitButton;

void setAI( bool _aienabled );
void initConsole( );
void consoleExecute( );
void consoleTab( );
void resumeButtonFunc( );
void exitButtonFunc( );
void initMenu( );
void uninitMenu( );
void reshape( int _width, int _height );
void display( );
//void idle( );
void gametimer( int value );
void aigametimer( int value );
void key( unsigned char key, int x, int y );
void keyup( unsigned char key, int x, int y );
void mouse( int button, int state, int x, int y );
void mousemotion( int x, int y );
int main( int argc, char* argv[] );

void setAI( bool _aienabled ) {
	aienabled = _aienabled;
}

void initConsole( ) {
	//consolefuncs[ "print-globals" ] = ConsolePrintGlobals_t( globalVariables );
	consolefuncs[ "load-map" ] = ConsolePass< std::string >::Func( boost::bind( setMap, boost::ref( game ), boost::ref( pathgraph ), boost::ref( view ), boost::ref( sound ), boost::ref( keyboardController ), boost::ref( aiControllers ), boost::ref( globalVariables ), boost::ref( gametime ), _1 ) );
	consolefuncs[ "add-player" ] = ConsolePass< std::string >::Func( boost::bind( addPlayer, boost::ref( game ), boost::ref( keyboardController ), boost::ref( globalVariables ), boost::ref( gametime ), _1 ) );
	consolefuncs[ "add-ai" ] = ConsolePass< std::string >::Func( boost::bind( addAI, boost::ref( game ), boost::ref( pathgraph ), boost::ref( aiControllers ), boost::ref( globalVariables ), boost::ref( gametime ), _1 ) );
	consolefuncs[ "remove-actor" ] = ConsolePass< std::string >::Func( boost::bind( removeActor, boost::ref( game ), boost::ref( aiControllers ), _1 ) );
	consolefuncs[ "set-ai" ] = ConsolePass< bool >::Func( setAI );
	consolefuncs[ "write-shadow-maps" ] = ConsolePass< std::string >::Func( boost::bind( &GameView::writeShadowMaps, boost::ref( view ), _1 ) );
	consolefuncs[ "set-glow" ] = ConsolePass< bool >::Func( boost::bind( &GameView::setGlow, boost::ref( view ), _1 ) );
	consolefuncs[ "set-distortion" ] = ConsolePass< bool >::Func( boost::bind( &GameView::setDistortion, boost::ref( view ), _1 ) );
	consolefuncs[ "set-dynamic-lighting" ] = ConsolePass< bool >::Func( boost::bind( &GameView::setDynamicLighting, boost::ref( view ), _1 ) );
	consolefuncs[ "set-draw-projectiles" ] = ConsolePass< bool >::Func( boost::bind( &GameView::setDrawProjectiles, boost::ref( view ), _1 ) );
	consolefuncs[ "set-draw-particles" ] = ConsolePass< bool >::Func( boost::bind( &GameView::setDrawParticles, boost::ref( view ), _1 ) );
	consolefuncs[ "set-ambient-intensity" ] = ConsolePass< float >::Func( boost::bind( &GameView::setAmbientIntensity, boost::ref( view ), _1 ) );
	consolefuncs[ "set-walk-animation-speed" ] = ConsolePass< float >::Func( boost::bind( &GameView::setWalkAnimationSpeed, boost::ref( view ), _1 ) );
	consolefuncs[ "set-first-person" ] = ConsolePass< bool >::Func( boost::bind( setFirstPerson, boost::ref( game ), boost::ref( view ), boost::ref( keyboardController ), _1 ) );
	consolefuncs[ "set-camera" ] = ConsolePass< ActorModel >::Func( boost::bind( setCamera, boost::ref( view ), boost::ref( sound ), _1 ), game );
	consolefuncs[ "set-camera-null" ] = ConsoleFunc( boost::bind( setCamera, boost::ref( view ), boost::ref( sound ), (ActorModel *)0 ) );
	consolefuncs[ "set-global" ] = ConsolePass2< std::string, float >::Func( boost::bind( setGlobal, boost::ref( game ), boost::ref( globalVariables ), _1, _2 ) );
	consolefuncs[ "get-global" ] = ConsolePass< std::string >::ReturnS( boost::bind( getGlobal, boost::ref( globalVariables ), _1 ) );
	consolefuncs[ "quit" ] = ConsoleFunc( exitButtonFunc );
	consolefuncs[ "exit" ] = ConsoleFunc( exitButtonFunc );
	executedcmdsiter = executedcmds.begin( );
}

void consoleExecute( ) {
	consoleoutput->AddText( ">" + consoleinput->GetText( ) );
	//consoleExecute( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, consoleoutput, gametime, consoleinput->GetText( ) );

	std::string str = consoleinput->GetText( ), cmdname;
	std::istringstream strstream( str );
	strstream >> cmdname;
	if( consolefuncs.find( cmdname ) != consolefuncs.end( ) ) {
		std::string output = consolefuncs[ cmdname ]( str );
		if( !output.empty( ) ) {
			consoleoutput->AddText( output );
		}
		executedcmds.push_back( str );
		executedcmdsiter = executedcmds.end( );
	} else {
		consoleoutput->AddText( "unrecognized command" );
	}

	consoleinput->GetText( ) = "";
}

void consoleTab( ) {
	//consoleTab( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, consoleinput, consoleoutput, gametime, consoleinput->GetText( ) );
	std::list< std::string > options = consoleTab( game, globalVariables, gametime, consoleinput->GetText( ) );

	std::string lcsstring = !options.empty( ) ? options.front( ) : std::string( );
	for( std::list< std::string >::iterator iter = options.begin( ); iter != options.end( ); ++iter ) {
		lcsstring = lcs( lcsstring, *iter );
		consoleoutput->AddText( *iter );
	}

	if( lcsstring.size( ) > consoleinput->GetText( ).size( ) ) {
		consoleinput->GetText( ) = lcsstring;
	}
}

void consolePageup( ) {
	console->SetActivated( consoleoutput );
}

void consoleUp( ) {
	if( executedcmdsiter != executedcmds.begin( ) ) {
		--executedcmdsiter;
	}

	consoleinput->GetText( ) = executedcmdsiter != executedcmds.end( ) ? *executedcmdsiter : std::string( );
}

void consoleDown( ) {
	if( executedcmdsiter != executedcmds.end( ) ) {
		++executedcmdsiter;
	}

	consoleinput->GetText( ) = executedcmdsiter != executedcmds.end( ) ? *executedcmdsiter : std::string( );
}

void resumeButtonFunc( ) {
	menuup = false;
}

void exitButtonFunc( ) {
	uninitMenu( );
	sound.uninit( );
	view.uninit( );

	exit( 0 );
}

void initMenu( ) {
	menuRenderer = new MenuRendererOpenGL( );
	menuRenderer->init( "./textures/letters.tga" );

	std::map< int, void (*)( ) > consolekeys;
	consolekeys[ 13 ] = consoleExecute;
	consolekeys[ 9 ] = consoleTab;
	consolekeys[ 256 ] = consoleUp;
	consolekeys[ 257 ] = consoleDown;
	consolekeys[ 258 ] = consolePageup;
	consolekeys[ 259 ] = consolePageup;
	consoleinput = new Menu::Textbox( Vector2( 1.5f - 1.0f / 32.0f, 1.0f / 24.0f ), consolekeys );
	consoleoutput = new Menu::MultiTextbox( Vector2( 1.5f - 1.0f / 32.0f, 1.5f - 1.0f / 32.0f - 1.0f / 64.0f - 1.0f / 24.0f ) );

	console = new Menu::Frame( Vector2( 1.5f, 1.5f ) );
	console->Add( Vector2( 1.0f / 64.0f, 1.0f / 64.0f ), consoleinput );
	console->Add( Vector2( 1.0f / 64.0f, 1.0f / 32.0f + 1.0f / 24.0f ), consoleoutput );

	resumeButton = new Menu::Button( "Resume", Vector2( 0.25f, 1.0f / 24.0f ), resumeButtonFunc );
	exitButton = new Menu::Button( "Exit", Vector2( 0.25f, 1.0f / 24.0f ), exitButtonFunc );

	menu = new Menu::Frame( Vector2( 1.0f, 1.0f ) );
	menu->Add( Vector2( 0.5f - 0.125f, 0.75f ), resumeButton );
	menu->Add( Vector2( 0.5f - 0.125f, 0.25f ), exitButton );
}

void uninitMenu( ) {
	delete menu;

	delete console;

	menuRenderer->uninit( );
	delete menuRenderer;
}

void reshape( int _width, int _height ) {
	glViewport( 0, 0, _width, _height );
	width = _width;
	height = _height;
}

void display( ) {
	view.draw( gametime );

	glDisable( GL_DEPTH_TEST );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	if( consoleup ) {
		console->Render( menuRenderer, Vector2( -0.75f, -0.75f ), glutGet( GLUT_ELAPSED_TIME ) / 1000.0f, false );
	}

	if( menuup ) {
		menu->Render( menuRenderer, Vector2( -0.5f, -0.5f ), glutGet( GLUT_ELAPSED_TIME ) / 1000.0f, false );
	}

	glutSwapBuffers( );
}

/*float lastlogicupdate = -1.0f;
float lastaiupdate = -1.0f;
float lastdraw = -1.0f;
void idle( ) {
	if( consoleup ) {
		gametime = glutGet( GLUT_ELAPSED_gametime ) / 1000.0f;
		if( gametime - lastdraw > 0.03f ) {
			glutPostRedisplay( );

			lastdraw = gametime;
		}
	} else {
		gametime = glutGet( GLUT_ELAPSED_gametime ) / 1000.0f;
		//float begingametime = glutGet( GLUT_ELAPSED_gametime ) / 1000.0f;

		if( gametime - lastlogicupdate > 0.03f ) {
			keyboardController->setMouse( ( mousex - width / 2.0f ) / ( width / 2.0f ), ( height / 2.0f - mousey ) / ( height / 2.0f ) );
			if( firstperson ) {
				glutWarpPointer( width / 2, height / 2 );
			}

			game.update( gametime );
			view.update( gametime );
			sound.update( gametime );

			lastlogicupdate = gametime;
		}

		if( gametime - lastaiupdate > 0.2f ) {
			aiController[ 0 ]->update( gametime );
			aiController[ 1 ]->update( gametime );
			aiController[ 2 ]->update( gametime );
			//aiController[ 3 ]->update( gametime );

			lastaiupdate = gametime;
		}

		//float endgametime = glutGet( GLUT_ELAPSED_gametime ) / 1000.0f;

		//gametime += endgametime - begingametime;

		if( gametime - lastdraw > 0.03f ) {
			glutPostRedisplay( );

			lastdraw = gametime;
		}
	}
}*/

// does logic updates
void gametimer( int value ) {
	if( consoleup || menuup ) {
	} else {
		keyboardController->setMouse( ( mousex - width / 2.0f ) / ( width / 2.0f ), ( height / 2.0f - mousey ) / ( height / 2.0f ) );
		if( dynamic_cast< ActorControllerKeyboardFP * >( keyboardController ) ) {
			glutWarpPointer( width / 2, height / 2 );
		}

		game.update( gametime );
		view.update( gametime );
		sound.update( gametime );

		gametime += 0.03f;
	}

	glutPostRedisplay( );

	glutTimerFunc( 30, gametimer, 0 );
}

// update AI agents
void aigametimer( int value ) {
	if( consoleup || menuup ) {
	} else {
		//static std::map< std::string, ActorControllerAI * >::iterator aiiter = aiControllers.begin( );
		if( aienabled ) {
			/*if( aiiter != aiControllers.end( ) ) {
				aiiter->second->update( gametime );
				++aiiter;
			}
			if( aiiter == aiControllers.end( ) ) {
				aiiter = aiControllers.begin( );
			}*/
			for( std::map< std::string, ActorControllerAI * >::iterator iter = aiControllers.begin( ); iter != aiControllers.end( ); ++iter ) {
				iter->second->update( gametime );
			}
		}
	}

	glutTimerFunc( 200, aigametimer, 0 );
}

void key( unsigned char key, int x, int y ) {
	if( consoleup ) {
		if( key == '`' ) {
			consoleup = false;
		} else {
			console->KeyPress( key );
		}
	} else if( menuup ) {
		if( key == 27 ) {
			menuup = false;
		} else {
			menu->KeyPress( key );
		}
	} else {
		if( key == '`' ) {
			consoleup = true;
			console->SetActivated( consoleinput );
		} else if( key == 27 ) {
			menuup = true;
			menu->SetActivated( 0 );
		} else {
			keyboardController->getKey( key ) = true;
		}
	}
}

void keyup( unsigned char key, int x, int y ) {
	keyboardController->getKey( key ) = false;
}

void special( int key, int x, int y ) {
	if( consoleup ) {
		if( key == GLUT_KEY_UP ) {
			console->KeyPress( 256 );
		} else if( key == GLUT_KEY_DOWN ) {
			console->KeyPress( 257 );
		} else if( key == GLUT_KEY_PAGE_UP ) {
			console->KeyPress( 258 );
		} else if( key == GLUT_KEY_PAGE_DOWN ) {
			console->KeyPress( 259 );
		}
	} else if( menuup ) {
		if( key == GLUT_KEY_UP ) {
			menu->KeyPress( 256 );
		} else if( key == GLUT_KEY_DOWN ) {
			menu->KeyPress( 257 );
		} else if( key == GLUT_KEY_PAGE_UP ) {
			menu->KeyPress( 258 );
		} else if( key == GLUT_KEY_PAGE_DOWN ) {
			menu->KeyPress( 259 );
		}
	}
}

void specialup( int key, int x, int y ) {
}

void mouse( int button, int state, int x, int y ) {
	if( consoleup ) {
		console->MouseClick( Vector2( 2.0f * x / 768.0f - 1.0f + 0.75f, 1.0f - 2.0f * y / 768.0f + 0.75f ) );
	}
	if( menuup ) {
		menu->MouseClick( Vector2( 2.0f * x / 768.0f - 1.0f + 0.5f, 1.0f - 2.0f * y / 768.0f + 0.5f ) );
	}

	keyboardController->getMouseButton( button ) = state == GLUT_DOWN ? true : false;
	mousex = x; mousey = y;
}

void mousemotion( int x, int y ) {
	mousex = x; mousey = y;
}

int main( int argc, char* argv[] ) {
	alutInit( &argc, argv );

	glutInit( &argc, argv );
	glutInitContextVersion( 3, 2 );
	glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize( width, height );
	glutInitWindowPosition( 50, 50 );

	glutCreateWindow( "Final Project" );

	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	
	glutReshapeFunc( reshape );
	glutDisplayFunc( display );
	glutKeyboardFunc( key );
	glutKeyboardUpFunc( keyup );
	glutSpecialFunc( special );
	glutSpecialUpFunc( specialup );
	glutMouseFunc( mouse );
	glutMotionFunc( mousemotion );
	glutPassiveMotionFunc( mousemotion );

	initExtensions( );

	// one time game initializations
	initMenu( );
	initConsole( );
	initGlobals( globalVariables );
	view.init( );
	sound.init( );
	keyboardController = new ActorControllerKeyboardTP( view );

	// setup game variables
	resetGameLoadMapAddActors( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, 0.0f, "./maps/test4.map" );

	glutTimerFunc( 0, gametimer, 0 );
	glutTimerFunc( 0, aigametimer, 0 );
	//glutIdleFunc( idle );
	glutMainLoop( );

	std::cout << "uninitializing" << std::endl;
	uninitMenu( );
	sound.uninit( );
	view.uninit( );

	return 0;
}
