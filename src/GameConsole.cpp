#include "GameConsole.h"

// longest common substring
std::string lcs( const std::string & str1, const std::string & str2 ) {
	size_t i;
	for( i = 0; i < str1.size( ) && i < str2.size( ) && str1[ i ] == str2[ i ]; ++i );
	return str1.substr( 0, i );
}

// execute console commands (bring down the console with the '`' key)
// console commands are displayed in the console window
/*void consoleExecute( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, Menu::MultiTextbox * consoleoutput, float gametime, const std::string & str ) {
	if( str.substr( 0, 13 ) == "print_globals" ) {
		ConsolePrintGlobals( globalVariables );
	} else if( str.substr( 0, 9 ) == "load_map " ) {
		ConsoleLoadMap( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, gametime, str );
	} else if( str.substr( 0, 11 ) == "add_player " ) {
		ConsoleAddPlayer( game, pathgraph, view, sound, keyboardController, globalVariables, gametime, str );
	} else if( str.substr( 0, 7 ) == "add_ai " ) {
		ConsoleAddAI( game, pathgraph, aiControllers, globalVariables, gametime, str );
	} else if( str.substr( 0, 13 ) == "remove_actor " ) {
		ConsoleRemoveActor( game, aiControllers, str );
	} else if( str.substr( 0, 4 ) == "set " ) {
		ConsoleSet( game, view, globalVariables, str );
	} else if( str.substr( 0, 4 ) == "get " ) {
		ConsoleGet( globalVariables, str );
	} else if( str.substr( 0, 11 ) == "set_camera " ) {
		ConsoleSetCamera( game, view, sound, str );
	} else if( str.substr( 0, 4 ) == "quit" ) {
		//uninitMenu( );
		sound.uninit( );
		view.uninit( );

		exit( 0 );
	} else if( str.substr( 0, 4 ) == "exit" ) {
		//uninitMenu( );
		sound.uninit( );
		view.uninit( );

		exit( 0 );
	} else {
		consoleoutput->AddText( "unrecognized command" );
	}
}*/

// give tab options
std::list< std::string > consoleTab( GameModel & game, std::map< std::string, float > & globalVariables, float gametime, const std::string & str ) {
	std::list< std::string > options;

	if( str.substr( 0, 9 ) == "load-map " ) {
		std::string cmdname, filename;

		std::istringstream strstream( str );
		strstream >> cmdname >> filename;

		boost::filesystem::directory_iterator end_itr;
		for( boost::filesystem::directory_iterator iter( boost::filesystem::path( "./maps/" ) ); iter != end_itr; ++iter ) {
			if( iter->filename( ).substr( 0, filename.size( ) ) == filename ) {
				if( iter->filename( ).rfind( "." ) != std::string::npos && iter->filename( ).substr( iter->filename( ).rfind( "." ) ) == ".map" ) {
					options.push_back( "load_map " + iter->filename( ) );
				}
			}
		}
	} else if( str.substr( 0, 13 ) == "remove-actor " ) {
		std::string cmdname, actorname;

		std::istringstream strstream( str );
		strstream >> cmdname >> actorname;

		for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
			if( iter->getName( ).substr( 0, actorname.size( ) ) == actorname ) {
				options.push_back( "remove_actor " + iter->getName( ) );
			}
		}
	} else if( str.substr( 0, 11 ) == "set-global " ) {
		std::string cmdname, varname;

		std::istringstream strstream( str );
		strstream >> cmdname >> varname;

		for( std::map< std::string, float >::iterator iter = globalVariables.begin( ); iter != globalVariables.end( ); ++iter ) {
			if( iter->first.substr( 0, varname.size( ) ) == varname ) {
				options.push_back( "set-global " + iter->first );
			}
		}
	} else if( str.substr( 0, 11 ) == "get-global " ) {
		std::string cmdname, varname;

		std::istringstream strstream( str );
		strstream >> cmdname >> varname;

		for( std::map< std::string, float >::iterator iter = globalVariables.begin( ); iter != globalVariables.end( ); ++iter ) {
			if( iter->first.substr( 0, varname.size( ) ) == varname ) {
				options.push_back( "get-global " + iter->first );
			}
		}
	} else if( str.substr( 0, 11 ) == "set-camera " ) {
		std::string cmdname, actorname;

		std::istringstream strstream( str );
		strstream >> cmdname >> actorname;

		for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
			if( iter->getName( ).substr( 0, actorname.size( ) ) == actorname ) {
				options.push_back( "set-camera " + iter->getName( ) );
			}
		}
	} else {
		const std::string validcommands [] = { "load-map", "add-player", "add-ai", "remove-actor", "set-ai", "write-shadow-map", "set-glow", "set-distortion", "set-dynamic-lighting", "set-draw-projectiles", "set-draw-particles", "set-ambient-intensity", "set-first-person", "set-camera", "set-camera-null", "set-global", "get-global", "quit", "exit" };
		//const std::string usage [] = { "", "[filename]", "[name]", "[name]", "[name]", "[variable]", "[variable] [value]", "", "" };

		for( int i = 0; i < 19; ++i ) {
			if( validcommands[ i ].substr( 0, str.size( ) ) == str ) {
				options.push_back( validcommands[ i ] );
			}
		}
	}

	return options;
}

/// refactoring

/*std::string ConsolePrintGlobals( std::map< std::string, float > & globalVariables ) {
	for( std::map< std::string, float >::iterator iter = globalVariables.begin( ); iter != globalVariables.end( ); ++iter ) {
		std::cout << iter->first << " " << iter->second << std::endl;
	}

	return std::string( );
}*/

/*std::string ConsoleLoadMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & str ) {
	std::string cmdname, filename;

	std::istringstream strstream( str );
	strstream >> cmdname >> filename;

	std::string newfilename = "./maps/" + filename;

	std::ifstream filestream( newfilename.c_str( ) );
	if( filestream ) {
		filestream.close( );

		resetGameLoadMapAddActors( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, gametime, newfilename );

		return std::string( );
	} else {
		return "file not found";
	}
}

std::string ConsoleAddPlayer( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & str ) {
	std::string cmdname, actorname;

	std::istringstream strstream( str );
	strstream >> cmdname >> actorname;

	for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
		if( iter->getName( ) == actorname ) {
			return actorname + " already exists";
		}
	}

	// add the player
	//addPlayer( game, keyboardController, globalVariables, gametime, actorname );
	addActor( game, keyboardController, globalVariables, gametime, actorname );
	view.setCamera( &game.getActors( ).back( ) );
	sound.setCamera( &game.getActors( ).back( ) );

	return std::string( );
}

std::string ConsoleAddAI( GameModel & game, PathGraph & pathgraph, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & str ) {
	std::string cmdname, actorname;

	std::istringstream strstream( str );
	strstream >> cmdname >> actorname;

	// check if an actor with this name exists
	for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
		if( iter->getName( ) == actorname ) {
			return actorname + " already exists";
		}
	}

	// add the actor
	aiControllers[ actorname ] = new ActorControllerAI( &game, pathgraph );
	//addAI( game, aiControllers[ actorname ], globalVariables, gametime, actorname );
	addActor( game, aiControllers[ actorname ], globalVariables, gametime, actorname );

	return std::string( );
}

std::string ConsoleRemoveActor( GameModel & game, std::map< std::string, ActorControllerAI * > & aiControllers, const std::string & str ) {
	std::string cmdname, actorname;

	std::istringstream strstream( str );
	strstream >> cmdname >> actorname;

	if( aiControllers.find( actorname ) != aiControllers.end( ) ) {
		delete aiControllers[ actorname ];
		aiControllers.erase( actorname );
		game.removeActor( actorname );
	} else {
		game.removeActor( actorname );
	}

	return std::string( );
}

std::string ConsoleSet( GameModel & game, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, float > & globalVariables, const std::string & str ) {
	std::string cmdname, varname;

	std::istringstream strstream( str );
	strstream >> cmdname >> varname;

	if( globalVariables.find( varname ) != globalVariables.end( ) ) {
		float value;
		strstream >> value;

		globalVariables[ varname ] = value;

		setGameVars( game, globalVariables );
		//initWeapons( globalVariables, knife, machinegun, rocketlauncher, shotgun );
		resetActorVars( game, globalVariables );

		return varname + " = " + toString( value );
	} else if( varname == "view_camera" ) {
		std::string actorname;
		strstream >> actorname;

		if( actorname == "null" ) {
			view.setCamera( 0 );
			sound.setCamera( 0 );
			return "camera set to null";
		} else {
			for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
				if( iter->getName( ) == actorname ) {
					view.setCamera( &(*iter) );
					sound.setCamera( &(*iter) );
					return "camera set to actor " + actorname;
				}
			}
			return std::string( );
		}
	} else if( varname == "view_first_person" ) {
		std::string value;
		strstream >> value;

		std::list< ActorModel >::iterator aiter = game.getActors( ).begin( );
		std::list< const ActorController * >::iterator citer = game.getActorControllers( ).begin( );
		for( ; aiter != game.getActors( ).end( ); ++citer, ++aiter ) {
			if( *citer == keyboardController ) {
				break;
			}
		}

		if( aiter != game.getActors( ).end( ) ) {
			if( value == "true" || value == "1" ) {
				delete keyboardController;
				keyboardController = new ActorControllerKeyboardFP( view );
				keyboardController->setModel( &(*aiter) );
				*citer = keyboardController;
				view.setFirstPerson( );
			}
			if( value == "false" || value == "0" ) {
				delete keyboardController;
				keyboardController = new ActorControllerKeyboardTP( view );
				keyboardController->setModel( &(*aiter) );
				*citer = keyboardController;
				view.setThirdPerson( );
			}
		}
		return std::string( );
	} else {
		return "unknown variable";
	}
}

std::string ConsoleGet( std::map< std::string, float > & globalVariables, const std::string & str ) {
	std::string cmdname, varname;

	std::istringstream strstream( str );
	strstream >> cmdname >> varname;

	if( globalVariables.find( varname ) != globalVariables.end( ) ) {
		//consoleoutput->AddText( varname + " = " + toString( globalVariables[ varname ] ) );
		return varname + " = " + toString( globalVariables[ varname ] );
	} else {
		//consoleoutput->AddText( "unknown variable" );
		return "unknown variable";
	}
}*/

/*ConsolePrintGlobals_t::ConsolePrintGlobals_t( std::map< std::string, float > & _globalVariables ) : globalVariables( _globalVariables ) {
}

std::string ConsolePrintGlobals_t::operator( )( float gametime, const std::string & str ) {
	return ConsolePrintGlobals( globalVariables );
}*/

/*ConsoleLoadMap_t::ConsoleLoadMap_t( GameModel & _game, PathGraph & _pathgraph, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, ActorControllerAI * > & _aiControllers, std::map< std::string, float > & _globalVariables ) : game( _game ), pathgraph( _pathgraph ), view( _view ), sound( _sound ), keyboardController( _keyboardController ), aiControllers( _aiControllers ), globalVariables( _globalVariables ) {
}

std::string ConsoleLoadMap_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleLoadMap( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, gametime, str );
}

ConsoleAddPlayer_t::ConsoleAddPlayer_t( GameModel & _game, PathGraph & _pathgraph, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, float > & _globalVariables ) : game( _game ), pathgraph( _pathgraph ), view( _view ), sound( _sound ), keyboardController( _keyboardController ), globalVariables( _globalVariables ) {
}

std::string ConsoleAddPlayer_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleAddPlayer( game, pathgraph, view, sound, keyboardController, globalVariables, gametime, str );
}

ConsoleAddAI_t::ConsoleAddAI_t( GameModel & _game, PathGraph & _pathgraph, std::map< std::string, ActorControllerAI * > & _aiControllers, std::map< std::string, float > & _globalVariables ) : game( _game ), pathgraph( _pathgraph ), aiControllers( _aiControllers ), globalVariables( _globalVariables ) {
}

std::string ConsoleAddAI_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleAddAI( game, pathgraph, aiControllers, globalVariables, gametime, str );
}

ConsoleRemoveActor_t::ConsoleRemoveActor_t( GameModel & _game, std::map< std::string, ActorControllerAI * > & _aiControllers ) : game( _game ), aiControllers( _aiControllers ) {
}

std::string ConsoleRemoveActor_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleRemoveActor( game, aiControllers, str );
}

ConsoleEnableBool_t::ConsoleEnableBool_t( bool & _val ) : val( _val ) { }

std::string ConsoleEnableBool_t::operator( )( float gametime, const std::string & str ) {
	val = true;
	return std::string( );
}

ConsoleDisableBool_t::ConsoleDisableBool_t( bool & _val ) : val( _val ) { }

std::string ConsoleDisableBool_t::operator( )( float gametime, const std::string & str ) {
	val = false;
	return std::string( );
}

ConsoleSet_t::ConsoleSet_t( GameModel & _game, GameView & _view, GameViewSound & _sound, ActorControllerKeyboard * & _keyboardController, std::map< std::string, float > & _globalVariables ) : game( _game ), view( _view ), sound( _sound ), keyboardController( _keyboardController ), globalVariables( _globalVariables ) {
}

std::string ConsoleSet_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleSet( game, view, sound, keyboardController, globalVariables, str );
}

ConsoleGet_t::ConsoleGet_t( std::map< std::string, float > & _globalVariables ) : globalVariables( _globalVariables ) {
}

std::string ConsoleGet_t::operator( )( float gametime, const std::string & str ) {
	return ConsoleGet( globalVariables, str );
}*/

