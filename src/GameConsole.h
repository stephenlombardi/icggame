#ifndef _GameConsole_h
#define _GameConsole_h

#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>

#include "GameModel.h"
#include "GameView.h"
#include "GameViewSound.h"
#include "Weapon.h"
#include "GameSetup.h"
#include "ActorModel.h"
#include "ActorController.h"
#include "ActorControllerKeyboard.h"
#include "ActorControllerKeyboardFP.h"
#include "ActorControllerKeyboardTP.h"
#include "ActorControllerAI.h"

template< class T >
std::string toString( T t ) {
	std::ostringstream stream;
	stream << t;
	return stream.str( );
}

/*template< class T >
void setBoolFromString( std::istream & stream, T func ) {
	std::string value;
	stream >> value;

	if( value == "true" || value == "1" ) {
		func( true );
	} else if( value == "false" || value == "0" ) {
		func( false );
	}
}*/

// longest common substring
std::string lcs( const std::string & str1, const std::string & str2 );

//void consoleExecute( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, Menu::MultiTextbox * consoleoutput, float gametime, const std::string & str );
std::list< std::string > consoleTab( GameModel & game, std::map< std::string, float > & globalVariables, float gametime, const std::string & str );

// refacotring
//std::string ConsolePrintGlobals( std::map< std::string, float > & globalVariables );
//std::string ConsoleLoadMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & str );
//std::string ConsoleAddPlayer( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & str );
//std::string ConsoleAddAI( GameModel & game, PathGraph & pathgraph, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & str );
//std::string ConsoleRemoveActor( GameModel & game, std::map< std::string, ActorControllerAI * > & aiControllers, const std::string & str );
//std::string ConsoleSet( GameModel & game, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, float > & globalVariables, const std::string & str );
//std::string ConsoleGet( std::map< std::string, float > & globalVariables, const std::string & str );

/*class ConsolePrintGlobals_t {
public:
	ConsolePrintGlobals_t( std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	std::map< std::string, float > & globalVariables;
};*/

/*class ConsoleLoadMap_t {
public:
	ConsoleLoadMap_t( GameModel & _game, PathGraph & _pathgraph, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, ActorControllerAI * > & _aiControllers, std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	GameModel & game;
	PathGraph & pathgraph;
	GameView & view;
	GameViewSound & sound;
	ActorControllerKeyboard * & keyboardController;
	std::map< std::string, ActorControllerAI * > & aiControllers;
	std::map< std::string, float > & globalVariables;
};

class ConsoleAddPlayer_t {
public:
	ConsoleAddPlayer_t( GameModel & _game, PathGraph & _pathgraph, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	GameModel & game;
	PathGraph & pathgraph;
	GameView & view;
	GameViewSound & sound;
	ActorControllerKeyboard * & keyboardController;
	std::map< std::string, float > & globalVariables;
};

class ConsoleAddAI_t {
public:
	ConsoleAddAI_t( GameModel & _game, PathGraph & _pathgraph, std::map< std::string, ActorControllerAI * > & _aiControllers, std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	GameModel & game;
	PathGraph & pathgraph;
	std::map< std::string, ActorControllerAI * > & aiControllers;
	std::map< std::string, float > & globalVariables;
};

class ConsoleRemoveActor_t {
public:
	ConsoleRemoveActor_t( GameModel & _game, std::map< std::string, ActorControllerAI * > & _aiControllers );
	std::string operator( )( float gametime, const std::string & str );
private:
	GameModel & game;
	std::map< std::string, ActorControllerAI * > & aiControllers;
};

class ConsoleEnableBool_t {
public:
	ConsoleEnableBool_t( bool & _val );
	std::string operator( )( float gametime, const std::string & str );
private:
	bool & val;
};

class ConsoleDisableBool_t {
public:
	ConsoleDisableBool_t( bool & _val );
	std::string operator( )( float gametime, const std::string & str );
private:
	bool & val;
};

class ConsoleSet_t {
public:
	ConsoleSet_t( GameModel & _game, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	GameModel & game;
	GameView & view;
	GameViewSound & sound;
	ActorControllerKeyboard * & keyboardController;
	std::map< std::string, float > & globalVariables;
};

class ConsoleGet_t {
public:
	ConsoleGet_t( std::map< std::string, float > & _globalVariables );
	std::string operator( )( float gametime, const std::string & str );
private:
	std::map< std::string, float > & globalVariables;
};*/

template< class FuncT >
class ConsoleFunc_t {
public:
	ConsoleFunc_t( FuncT _func ) : func( _func ) { }
	std::string operator( )( const std::string & str ) {
		func( );
		return std::string( );
	}
private:
	FuncT func;
};

template< class FuncT >
ConsoleFunc_t< FuncT > ConsoleFunc( FuncT func ) {
	return ConsoleFunc_t< FuncT >( func );
}

template< class FuncT, class T >
class ConsoleFuncPassTReturnS_t {
public:
	ConsoleFuncPassTReturnS_t( FuncT _func ) : func( _func ) { }
	std::string operator( )( const std::string & str ) {
		std::string cmdname;
		T option;
		std::istringstream strstream( str );
		strstream >> cmdname >> option;
		if( !strstream.fail( ) ) {
			return func( option );
		} else {
			return "invalid input";
		}
	}
private:
	FuncT func;
};

template< class FuncT, class T >
class ConsoleFuncPassT_t {
public:
	ConsoleFuncPassT_t( FuncT _func ) : func( _func ) { }
	std::string operator( )( const std::string & str ) {
		std::string cmdname;
		T option;
		std::istringstream strstream( str );
		strstream >> cmdname >> option;
		if( !strstream.fail( ) ) {
			func( option );
			return str;
		} else {
			return "invalid input";
		}
	}
private:
	FuncT func;
};

template< class FuncT >
class ConsoleFuncPassT_t< FuncT, ActorModel > {
public:
	ConsoleFuncPassT_t( FuncT _func, const GameModel & _game ) : func( _func ), game( _game ) { }
	std::string operator( )( const std::string & str ) {
		std::string cmdname, actorname;
		std::istringstream strstream( str );
		strstream >> cmdname >> actorname;
		if( !strstream.fail( ) ) {
			for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
				if( iter->getName( ) == actorname ) {
				//if( iter->getName( ).substr( 0, actorname.size( ) ) == actorname ) {
					func( &(*iter) );
					break;
				}
			}
			return str;
		} else {
			return "invalid input";
		}
	}
private:
	FuncT func;
	const GameModel & game;
};

template< class T >
class ConsolePass {
public:
	template< class FuncT >
	static ConsoleFuncPassTReturnS_t< FuncT, T > ReturnS( FuncT func ) {
		return ConsoleFuncPassTReturnS_t< FuncT, T >( func );
	}

	template< class FuncT >
	static ConsoleFuncPassT_t< FuncT, T > Func( FuncT func ) {
		return ConsoleFuncPassT_t< FuncT, T >( func );
	}
private:
};

template< class FuncT, class T1, class T2 >
class ConsoleFuncPassT2_t {
public:
	ConsoleFuncPassT2_t( FuncT _func ) : func( _func ) { }
	std::string operator( )( const std::string & str ) {
		std::string cmdname;
		T1 option1;
		T2 option2;
		std::istringstream strstream( str );
		strstream >> cmdname >> option1 >> option2;
		if( !strstream.fail( ) ) {
			func( option1, option2 );
			return str;
		} else {
			return "invalid input";
		}
	}
private:
	FuncT func;
};

template< class T1, class T2 >
class ConsolePass2 {
public:
	template< class FuncT >
	static ConsoleFuncPassT2_t< FuncT, T1, T2 > Func( FuncT func ) {
		return ConsoleFuncPassT2_t< FuncT, T1, T2 >( func );
	}
};

/*template< class FuncT >
class ConsoleFuncPassT_t< FuncT, bool > {
public:
	ConsoleFuncPassT_t( FuncT _func ) : func( _func ) { }
	std::string operator( )( GameModel & game, PathGraph & pathgraph, float gametime, const std::string & str ) {
		std::string cmdname, option;
		std::istringstream strstream( str );
		strstream >> cmdname >> option;
		if( option == "true" || option == "1" ) {
			func( true );
		}
		if( option == "false" || option == "0" ) {
			func( false );
		}
		return std::string( );
	}
private:
	FuncT func;
};*/

/*template< class FuncT, class T >
ConsoleFuncPassT_t< FuncT, T > ConsoleFuncPassT( FuncT func, T t ) {
	return ConsoleFuncPassT_t< FuncT, T >( func );
}*/

/*template< class T >
class ConsoleFuncPassBool_t {
public:
	ConsoleFuncPassBool_t( T _t ) : t( _t ) { }
	std::string operator( )( GameModel & game, PathGraph & pathgraph, float gametime, const std::string & str ) {
		std::string cmdname, option;
		std::istringstream strstream( str );
		strstream >> cmdname >> option;
		if( option == "true" || option == "1" ) {
			t( true );
		}
		if( option == "false" || option == "0" ) {
			t( false );
		}
		return std::string( );
	}
private:
	T t;
};

template< class T >
ConsoleFuncPassBool_t< T > ConsoleFuncPassBool( T t ) {
	return ConsoleFuncPassBool_t< T >( t );
}*/

#endif
