#include "GameSetup.h"

void initGlobals( std::map< std::string, float > & globalVariables ) {
	std::ifstream gamevars( "gamevars.txt" );

	while( gamevars ) {
		std::string varname;
		float varvalue;
		gamevars >> varname >> varvalue;

		if( gamevars ) {
			globalVariables[ varname ] = varvalue;
		}
	}

	std::clog << globalVariables.size( ) << " gamevars parsed" << std::endl;
	
	// todo: check validity..
}

void resetGameLoadMapAddActors( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & filename ) {
	// delete ai controllers
	for( std::map< std::string, ActorControllerAI * >::iterator iter = aiControllers.begin( ); iter != aiControllers.end( ); iter++ ) {
		delete iter->second;
	}
	aiControllers.clear( );

	// reset game
	resetGame( game, pathgraph, view, sound );
	setGameVars( game, globalVariables );

	// load map
	loadMap( game, pathgraph, view, sound, globalVariables, filename );
	view.initBrushTextures( );

	// add players
	//addPlayer( game, keyboardController, globalVariables, gametime, "Player" );
	addActor( game, keyboardController, globalVariables, gametime, "Player" );
	for( int i = 0; i < 3; i++ ) {
		aiControllers[ "AI-" + std::string( 1, (char)( i + 49 ) ) ] = new ActorControllerAI( &game, pathgraph );
		//addAI( game, aiControllers[ "AI-" + std::string( 1, (char)( i + 49 ) ) ], globalVariables, gametime, "AI-" + std::string( 1, (char)( i + 49 ) ) );
		addActor( game, aiControllers[ "AI-" + std::string( 1, (char)( i + 49 ) ) ], globalVariables, gametime, "AI-" + std::string( 1, (char)( i + 49 ) ) );
	}
	view.setCamera( &game.getActors( ).front( ) );
	sound.setCamera( &game.getActors( ).front( ) );
}

void resetGame( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound ) {
	view.clearObjects( );
	view.uninitActorText( );
	view.uninitBrushes( );

	sound.uninitActors( );

	game = GameModel( );
	pathgraph = PathGraph( );

	game.addObserver( &view );
	view.setGame( &game );
	view.setThirdPerson( );

	game.addObserver( &sound );
	sound.setGame( &game );
}

void setGameVars( GameModel & game, std::map< std::string, float > & globalVariables ) {
	game.setDeathTime( globalVariables[ "actor_deathtime" ] );
	game.setActorMaxHealth( globalVariables[ "actor_maxhealth" ] );
	game.setItemRespawnTime( globalVariables[ "item_respawntime" ] );
	game.setItemBoundingRadius( globalVariables[ "item_size" ] );
}

void loadMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, std::map< std::string, float > & globalVariables, const std::string & filename ) {
	std::ifstream mapfile( filename.c_str( ) );
	while( !mapfile.eof( ) ) {
		char c = Map::Consume( mapfile, "{" );
		if( !mapfile.eof( ) ) {
			mapfile.putback( c );
			Map::Entity entity;
			mapfile >> entity;

			if( entity.values[ "classname" ] == "worldspawn" ) {
				addWorldspawn( game, view, entity );
			} else if( entity.values[ "classname" ] == "info_player_start" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;
				game.addSpawnPoint( Vector3( x, y, -z ) );
			} else if( entity.values[ "classname" ] == "light" ) {
				vec3f lightpos;
				if( entity.values.find( "origin" ) != entity.values.end( ) ) {
					std::istringstream originstream( entity.values[ "origin" ] );
					float x, y, z;
					originstream >> x >> z >> y;
					lightpos = Vector3( x, y, -z );
				} else {
					lightpos = Vector3( 0.0f, 0.0f, 0.0f );
				}

				float lightpower;
				if( entity.values.find( "light" ) != entity.values.end( ) ) {
					std::istringstream lightpowerstream( entity.values[ "light" ] );
					lightpowerstream >> lightpower;
				} else {
					lightpower = 4.0f;
				}

				float lightatten;
				if( entity.values.find( "attenuation" ) != entity.values.end( ) ) {
					std::istringstream lightpowerstream( entity.values[ "attenuation" ] );
					lightpowerstream >> lightatten;
				} else {
					lightatten = 32.0f;
				}
				
				vec3f lightcolor;
				if( entity.values.find( "color" ) != entity.values.end( ) ) {
					std::istringstream colorstream( entity.values[ "color" ] );
					float r, g, b;
					colorstream >> r >> g >> b;
					lightcolor = Vector3( r, g, b );
				} else {
					lightcolor = Vector3( 1.0f, 1.0f, 1.0f );
				}

				vec3f lightdir;
				if( entity.values.find( "direction" ) != entity.values.end( ) ) {
					std::istringstream directionstream( entity.values[ "direction" ] );
					float x, y, z;
					directionstream >> x >> y >> z;
					lightdir = Vector3N( x, y, z );
				}

				if( entity.values.find( "direction" ) != entity.values.end( ) ) {
					if( entity.values.find( "directional" ) != entity.values.end( ) ) {
						view.addDirectionalLight( DirectionalLight( lightdir, lightcolor, lightpower ) );
					} else {
						view.addSpotLight( SpotLight( lightpos, lightdir, lightcolor, 3.1415926f / 2.0f, lightpower, 1.0f / ( lightatten * lightatten ) ) );
					}
				} else {
					view.addPointLight( PointLight( lightpos, lightcolor, lightpower, 1.0f / ( lightatten * lightatten ) ) );
				}

				//view.addLight( Vector3( x, y, -z ), Vector3( 1.0f, 1.0f, 1.0f ), lightpower );
			} else if( entity.values[ "classname" ] == "trigger_multiple" ) {
				std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > brushes;
				std::transform( entity.brushes.begin( ), entity.brushes.end( ), std::back_inserter( brushes ), mapBrushToPlanarVolume );
				game.addTrigger( Trigger( brushes ) );
			} else if( entity.values[ "classname" ] == "item_health" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				//game.addHealthSpawn( Vector3( x, y, -z ) );
				game.addPowerupSpawn( Vector3( x, y, -z ), "health" );
			} else if( entity.values[ "classname" ] == "powerup_homing" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addPowerupSpawn( Vector3( x, y, -z ), "homing" );
			} else if( entity.values[ "classname" ] == "powerup_attackspeed" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addPowerupSpawn( Vector3( x, y, -z ), "attackspeed" );
			} else if( entity.values[ "classname" ] == "powerup_speed" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addPowerupSpawn( Vector3( x, y, -z ), "speed" );
			} else if( entity.values[ "classname" ] == "weapon_knife" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), createKnife( globalVariables ) );
			} else if( entity.values[ "classname" ] == "weapon_machinegun" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), createMachinegun( globalVariables ) );
			} else if( entity.values[ "classname" ] == "weapon_shotgun" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), createShotgun( globalVariables ) );
			} else if( entity.values[ "classname" ] == "weapon_rocketlauncher" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), createRocketlauncher( globalVariables ) );
			/*} else if( entity.values[ "classname" ] == "weapon_grenadelauncher" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), grenadelauncher );
			} else if( entity.values[ "classname" ] == "weapon_flamethrower" ) {
				std::istringstream originstream( entity.values[ "origin" ] );
				float x, y, z;
				originstream >> x >> z >> y;

				game.addWeaponSpawn( Vector3( x, y, -z ), flamethrower );*/
			}
		}
	}
	mapfile.close( );

	std::clog << "map " << filename << " loaded" << std::endl;

	constructPathGraph( game, pathgraph );

	std::clog << "path graph created" << std::endl;
}

PlanarVolume< 3, float, TexturedPlane< 3, float > > mapBrushToPlanarVolume( const Map::Brush & mapbrush ) {
	std::list< TexturedPlane< 3, float > > planes;

	for( std::list< Map::BrushPlane >::const_iterator piter = mapbrush.planes.begin( ); piter != mapbrush.planes.end( ); piter++ ) {
		const Map::BrushPlane & mapbrushplane = *piter;

		vec3f point1 = Vector3( (float)mapbrushplane.planepoints[ 0 ], (float)mapbrushplane.planepoints[ 2 ], (float)-mapbrushplane.planepoints[ 1 ] );
		vec3f point2 = Vector3( (float)mapbrushplane.planepoints[ 3 ], (float)mapbrushplane.planepoints[ 5 ], (float)-mapbrushplane.planepoints[ 4 ] );
		vec3f point3 = Vector3( (float)mapbrushplane.planepoints[ 6 ], (float)mapbrushplane.planepoints[ 8 ], (float)-mapbrushplane.planepoints[ 7 ] );
		vec3f vec1 = point2 - point1;
		vec3f vec2 = point3 - point1;
		vec3f normal = Normalize( CrossProduct( vec2, vec1 ) );
		float distance = DotProduct( normal, point1 );

		planes.push_back( TexturedPlane< 3, float >( normal, distance ) );
	}

	return PlanarVolume< 3, float, TexturedPlane< 3, float > >( planes.begin( ), planes.end( ) );
}

void addWorldspawn( GameModel & game, GameView & view, Map::Entity & entity ) {
	std::map< std::string, std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > > texpolygons;
	for( std::list< Map::Brush >::iterator biter = entity.brushes.begin( ); biter != entity.brushes.end( ); biter++ ) {
		Map::Brush & mapbrush = *biter;

		/*std::list< TexturedPlane< 3, float > > planes;

		for( std::list< Map::BrushPlane >::iterator piter = mapbrush.planes.begin( ); piter != mapbrush.planes.end( ); piter++ ) {
			Map::BrushPlane & mapbrushplane = *piter;

			vec3f point1 = Vector3( (float)mapbrushplane.planepoints[ 0 ], (float)mapbrushplane.planepoints[ 2 ], (float)-mapbrushplane.planepoints[ 1 ] );
			vec3f point2 = Vector3( (float)mapbrushplane.planepoints[ 3 ], (float)mapbrushplane.planepoints[ 5 ], (float)-mapbrushplane.planepoints[ 4 ] );
			vec3f point3 = Vector3( (float)mapbrushplane.planepoints[ 6 ], (float)mapbrushplane.planepoints[ 8 ], (float)-mapbrushplane.planepoints[ 7 ] );
			vec3f vec1 = point2 - point1;
			vec3f vec2 = point3 - point1;
			vec3f normal = Normalize( CrossProduct( vec2, vec1 ) );
			float distance = DotProduct( normal, point1 );

			planes.push_back( TexturedPlane< 3, float >( normal, distance ) );
		}

		game.getBrushes( ).push_back( PlanarVolume< 3, float, TexturedPlane< 3, float > >( planes.begin( ), planes.end( ) ) );*/
		game.getBrushes( ).push_back( mapBrushToPlanarVolume( mapbrush ) );

		//std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > polygons = game.getBrushes( ).back( ).GeneratePolygons( );
		std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > polygons = GeneratePolygons( game.getBrushes( ).back( ) );

		std::list< Map::BrushPlane >::iterator planeiter = mapbrush.planes.begin( );
		for( std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > >::iterator piter = polygons.begin( ); piter != polygons.end( ); piter++, planeiter++ ) {
			PlanarPolygon< 3, float, TexturedPlane< 3, float > > & polygon = *piter;
			Map::BrushPlane & mapbrushplane = *planeiter;

			texpolygons[ mapbrushplane.texturepath ].push_back( polygon );
		}
	}

	game.initKdTree( );

	view.setBrushes( texpolygons );
}

void constructPathGraph( GameModel & game, PathGraph & pathgraph ) {
	vec3f min( std::numeric_limits< float >::infinity( ) ), max( -std::numeric_limits< float >::infinity( ) );
	for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::iterator iter = game.getBrushes( ).begin( ); iter != game.getBrushes( ).end( ); iter++ ) {
		PlanarVolume< 3, float, TexturedPlane< 3, float > > & brush = *iter;

		//std::list< vec3f > points = brush.GeneratePoints( );
		std::list< vec3f > points = GeneratePoints( brush );

		for( std::list< vec3f >::iterator iter2 = points.begin( ); iter2 != points.end( ); iter2++ ) {
			vec3f & point = *iter2;

			if( point[ 0 ] < min[ 0 ] ) {
				min[ 0 ] = point[ 0 ];
			}
			if( point[ 2 ] < min[ 2 ] ) {
				min[ 2 ] = point[ 2 ];
			}
			if( point[ 0 ] > max[ 0 ] ) {
				max[ 0 ] = point[ 0 ];
			}
			if( point[ 2 ] > max[ 2 ] ) {
				max[ 2 ] = point[ 2 ];
			}
		}
	}

	int gridwidth = 32;

	int startx = (int)min[ 0 ] / gridwidth, startz = (int)min[ 2 ] / gridwidth;
	int endx = (int)max[ 0 ] / gridwidth, endz = (int)max[ 2 ] / gridwidth;

	for( int z = startz; z <= endz; z++ ) {
		for( int x = startx; x <= endx; x++ ) {
			vec3f vertex = Vector3( (float)( x * gridwidth ), 16.0f, (float)( z * gridwidth ) );

			bool intersection = false;
			for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::iterator iter = game.getBrushes( ).begin( ); iter != game.getBrushes( ).end( ); iter++ ) {
				PlanarVolume< 3, float, TexturedPlane< 3, float > > & plane = *iter;
				const PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > & planeintersection = plane.IntersectLine( vertex, vertex, SphereOffsetCalc< 3, float >( 16.0f ) );
				if( intersection = planeintersection.enterfraction == -std::numeric_limits< float >::infinity( ) ) {
					break;
				}
			}

			if( !intersection ) {
				pathgraph.addVertex( vertex );
			}
		}
	}

	for( int i = 0; i < pathgraph.numVertices( ) - 1; i++ ) {
		for( int j = i + 1; j < pathgraph.numVertices( ); j++ ) {
			vec3f vertex1 = pathgraph.getVertexPosition( i ), vertex2 = pathgraph.getVertexPosition( j );

			if( Norm( vertex1 - vertex2 ) <= sqrt( 2.0f ) * gridwidth ) {
				pathgraph.addEdge( i, j );
			}
		}
	}

	//pathgraph.initShortestPath( );
}

/*Actor * addPlayer( GameModel & game, ActorControllerKeyboard * keyboardController, std::map< std::string, float > & globalVariables, const std::string & name ) {
	game.getActors( ).push_back( Actor( keyboardController, ActorModel( name, vec3f( 0.0f ), globalVariables[ "actor_maxhealth" ], globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) ) );
	game.spawnActor( game.getActors( ).back( ), 0.0f );
	keyboardController->setModel( &game.getActors( ).back( ) );
	return &game.getActors( ).back( );
}

Actor * addAI( GameModel & game, ActorControllerAI * aiController, std::map< std::string, float > & globalVariables, const std::string & name ) {
	game.getActors( ).push_back( Actor( aiController, ActorModel( name, vec3f( 0.0f ), globalVariables[ "actor_maxhealth" ], globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) ) );
	game.spawnActor( game.getActors( ).back( ), 0.0f );
	aiController->setModel( &game.getActors( ).back( ) );
	return &game.getActors( ).back( );
}*/

/*void addPlayer( GameModel & game, ActorControllerKeyboard * keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & name ) {
	//game.addActor( Actor( keyboardController, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) ) );
	game.addActor( keyboardController, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) );
	game.spawnActor( game.getActors( ).back( ), gametime );
	keyboardController->setModel( &game.getActors( ).back( ) );
	addWeapons( game.getActors( ).back( ), globalVariables );
	//return &game.getActors( ).back( );
}*/

/*void addAI( GameModel & game, ActorControllerAI * aiController, std::map< std::string, float > & globalVariables, float gametime, const std::string & name ) {
	//game.addActor( Actor( aiController, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) ) );
	game.addActor( aiController, ActorModel( name, globalVariables[ "actor_mass" ], globalVariables[ "actor_size" ], globalVariables[ "actor_speed" ] ) );
	game.spawnActor( game.getActors( ).back( ), gametime );
	aiController->setModel( &game.getActors( ).back( ) );
	addWeapons( game.getActors( ).back( ), globalVariables );
	//return &game.getActors( ).back( );
}*/

void addWeapons( ActorModel & actor, std::map< std::string, float > & globalVariables ) {
	actor.addWeapon( createKnife( globalVariables ) );
	actor.addWeapon( createMachinegun( globalVariables ) );
	actor.addWeapon( createRocketlauncher( globalVariables ) );
	actor.addWeapon( createShotgun( globalVariables ) );
	actor.switchWeapon( 1 );
}

/*void removeWeapons( ActorModel & actor ) {
	actor.removeWeapons( );
}*/

void resetActorVars( GameModel & game, std::map< std::string, float > & globalVariables ) {
	for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); iter++ ) {
		ActorModel & actor = *iter;

		//removeWeapons( actor );
		actor.removeWeapons( );
		addWeapons( actor, globalVariables );

		actor.getMass( ) = globalVariables[ "actor_mass" ];
		actor.getBoundingRadius( ) = globalVariables[ "actor_size" ];
		actor.getSpeed( ) = globalVariables[ "actor_speed" ];
	}
}

Weapon createKnife( std::map< std::string, float > & globalVariables ) {
	return Weapon( new ProjectileMelee( globalVariables[ "knife_mass" ], globalVariables[ "knife_size" ], globalVariables[ "knife_damage" ], globalVariables[ "knife_lifetime" ] ), globalVariables[ "knife_attackdelay" ], globalVariables[ "knife_muzzlespeed" ], globalVariables[ "knife_spread" ], (int)globalVariables[ "knife_projectiles" ] );
}

Weapon createMachinegun( std::map< std::string, float > & globalVariables ) {
	return Weapon( new ProjectileBullet( globalVariables[ "machinegun_mass" ], globalVariables[ "machinegun_size" ], globalVariables[ "machinegun_damage" ] ), globalVariables[ "machinegun_attackdelay" ], globalVariables[ "machinegun_muzzlespeed" ], globalVariables[ "machinegun_spread" ], (int)globalVariables[ "machinegun_projectiles" ] );
}

Weapon createRocketlauncher( std::map< std::string, float > & globalVariables ) {
	return Weapon( new ProjectileRocket( globalVariables[ "rocketlauncher_mass" ], globalVariables[ "rocketlauncher_size" ], globalVariables[ "rocketlauncher_damage" ], new EquationGaussian( globalVariables[ "rocketlauncher_splash_force" ], globalVariables[ "rocketlauncher_splash_force_size" ] ), new EquationGaussian( globalVariables[ "rocketlauncher_splash_damage" ], globalVariables[ "rocketlauncher_splash_damage_size" ] ) ), globalVariables[ "rocketlauncher_attackdelay" ], globalVariables[ "rocketlauncher_muzzlespeed" ], globalVariables[ "rocketlauncher_spread" ], (int)globalVariables[ "rocketlauncher_projectiles" ] );
}

Weapon createShotgun( std::map< std::string, float > & globalVariables ) {
	return Weapon( new ProjectileBounceLaser( globalVariables[ "shotgun_mass" ], globalVariables[ "shotgun_size" ], globalVariables[ "shotgun_damage" ] ), globalVariables[ "shotgun_attackdelay" ], globalVariables[ "shotgun_muzzlespeed" ], globalVariables[ "shotgun_spread" ], (int)globalVariables[ "shotgun_projectiles" ] );
}

///////////////////// extra /////////////////////////
void setFirstPerson( GameModel & game, GameView & view, ActorControllerKeyboard * & keyboardController, bool firstperson ) {
	std::list< ActorModel >::iterator aiter = game.getActors( ).begin( );
	std::list< const ActorController * >::iterator citer = game.getActorControllers( ).begin( );
	for( ; aiter != game.getActors( ).end( ); ++citer, ++aiter ) {
		if( *citer == keyboardController ) {
			break;
		}
	}

	if( aiter != game.getActors( ).end( ) ) {
		if( firstperson ) {
			delete keyboardController;
			keyboardController = new ActorControllerKeyboardFP( view );
			keyboardController->setModel( &(*aiter) );
			*citer = keyboardController;
			view.setFirstPerson( );
		} else {
			delete keyboardController;
			keyboardController = new ActorControllerKeyboardTP( view );
			keyboardController->setModel( &(*aiter) );
			*citer = keyboardController;
			view.setThirdPerson( );
		}
	}
}

void setGlobal( GameModel & game, std::map< std::string, float > & globalVariables, const std::string & varname, float value ) {
	if( globalVariables.find( varname ) != globalVariables.end( ) ) {
		//float value;
		//strstream >> value;

		globalVariables[ varname ] = value;

		setGameVars( game, globalVariables );
		//initWeapons( globalVariables, knife, machinegun, rocketlauncher, shotgun );
		resetActorVars( game, globalVariables );

		//return varname + " = " + toString( value );
	} else {
		//return "unknown variable";
	}
}

std::string getGlobal( std::map< std::string, float > & globalVariables, const std::string & varname ) {
	if( globalVariables.find( varname ) != globalVariables.end( ) ) {
		std::ostringstream strstream;
		strstream << globalVariables[ varname ];
		return strstream.str( );
	} else {
		return std::string( );
	}
}

void setCamera( GameView & view, GameViewSound & sound, const ActorModel * actor ) {
	view.setCamera( actor );
	sound.setCamera( actor );
}

void setMap( GameModel & game, PathGraph & pathgraph, GameView & view, GameViewSound & sound, ActorControllerKeyboard * & keyboardController, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & filename ) {
	std::string newfilename = "./maps/" + filename;

	std::ifstream filestream( newfilename.c_str( ) );
	if( filestream ) {
		filestream.close( );

		resetGameLoadMapAddActors( game, pathgraph, view, sound, keyboardController, aiControllers, globalVariables, gametime, newfilename );

		//return std::string( );
	} else {
		//return "file not found";
	}
}

void addPlayer( GameModel & game, ActorControllerKeyboard * keyboardController, std::map< std::string, float > & globalVariables, float gametime, const std::string & actorname ) {
	// check if an actor with this name exists
	for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
		if( iter->getName( ) == actorname ) {
			//return actorname + " already exists";
			return;
		}
	}

	addActor( game, keyboardController, globalVariables, gametime, actorname );
}

void addAI( GameModel & game, PathGraph & pathgraph, std::map< std::string, ActorControllerAI * > & aiControllers, std::map< std::string, float > & globalVariables, float gametime, const std::string & actorname ) {
	// check if an actor with this name exists
	for( std::list< ActorModel >::iterator iter = game.getActors( ).begin( ); iter != game.getActors( ).end( ); ++iter ) {
		if( iter->getName( ) == actorname ) {
			//return actorname + " already exists";
			return;
		}
	}

	aiControllers[ actorname ] = new ActorControllerAI( &game, pathgraph );
	addActor( game, aiControllers[ actorname ], globalVariables, gametime, actorname );
}

void removeActor( GameModel & game, std::map< std::string, ActorControllerAI * > & aiControllers, const std::string & actorname ) {
	if( aiControllers.find( actorname ) != aiControllers.end( ) ) {
		delete aiControllers[ actorname ];
		aiControllers.erase( actorname );
		game.removeActor( actorname );
	} else {
		game.removeActor( actorname );
	}
}
