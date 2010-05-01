#include "ActorControllerAI.h"

ActorControllerAI::ActorControllerAI( const GameModel * _game, const PathGraph & _pathgraph ) : game( _game ), pathgraph( _pathgraph ), actormodel( 0 ), nextvertex( -1 ), greatestthreat( 0.0f ), attacking( false ), weaponselection( 0 ) { }

vec3f ActorControllerAI::getMoveDirection( ) const {
	if( nextvertex > -1 ) {
		vec3f dir = pathgraph.getVertexPosition( nextvertex ) - actormodel->getPosition( );
		if( Norm( dir ) > 0.0f ) {
			return Normalize( dir );
		} else {
			return dir;
		}
	} else {
		return Vector3( 0.0f, 0.0f, 0.0f );
	}
}

vec3f ActorControllerAI::getLookDirection( ) const {
	//assert( Norm( greatestthreat - actormodel->getPosition( ) ) > 0.0f );
	if( Norm( greatestthreat - actormodel->getPosition( ) ) <= 1.0f / 1024.0f ) {
		if( Norm( actormodel->getVelocity( ) ) <= 1.0f / 1024.0f ) {
			return Vector3( 1.0f, 0.0f, 0.0f );
		} else {
			return Normalize( actormodel->getVelocity( ) );
		}
	} else {
		if( attacking && greatestthreat[ 0 ] < std::numeric_limits< float >::infinity( ) ) {
			return Normalize( greatestthreat - actormodel->getPosition( ) );
		} else {
			if( Norm( actormodel->getVelocity( ) ) <= 1.0f / 1024.0f ) {
				return Vector3( 1.0f, 0.0f, 0.0f );
			} else {
				return Normalize( actormodel->getVelocity( ) );
			}
		}
	}
}

int ActorControllerAI::getWeaponSwitch( ) const {
	return weaponselection;
}

bool ActorControllerAI::isAttacking( ) const {
	return attacking;
}

void ActorControllerAI::setModel( const ActorModel * _actormodel ) {
	actormodel = _actormodel;
}

float ActorControllerAI::calculateEnergy( int vertex ) const {
	float energy = 0.0f;

	vec3f vpos = pathgraph.getVertexPosition( vertex );

	for( std::list< Projectile * >::const_iterator iter = game->getProjectiles( ).begin( ); iter != game->getProjectiles( ).end( ); ++iter ) {
		const Projectile * projectile = *iter;

		if( !projectile->getOwner( ) || projectile->getOwner( ) != actormodel ) {
			vec3f pvec = vpos - projectile->getPosition( );
			float pvecnorm = Norm( pvec );
			vec3f pvecnormalized = pvec * ( 1.0f / pvecnorm );
			float angle = DotProduct( Normalize( projectile->getVelocity( ) ), pvecnormalized );

			//energy += -pvecnorm * std::max( angle, 0.0f );
			energy += ( 1024.0f / pvecnorm ) * std::max( angle, 0.0f );
		}
	}

	return energy;
}

vec3f ActorControllerAI::calculateThreat( ) const {
	// temp
	//return game->getActors( ).front( ).getPosition( );

	float closestVisibleDist = std::numeric_limits< float >::infinity( );
	const ActorModel * closestVisibleActor = 0;
	float closestDist = std::numeric_limits< float >::infinity( );
	const ActorModel * closestActor = 0;
	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;

		//if( actormodel != &actor && actor.getState( ) == "idle" ) {
		if( actormodel != &actor && actor.getHealth( ) > 0.0f ) {

			float closestT;
			const PlanarVolume< 3, float, TexturedPlane< 3, float > > * collidedBrush;
			const TexturedPlane< 3, float > * collidedPlane;
			game->getClosestBrushCollision( actormodel->getPosition( ), actor.getPosition( ) - actormodel->getPosition( ), 1.0f, closestT, collidedBrush, collidedPlane );

			if( closestT > 1.0f ) {
				if( Norm( actor.getPosition( ) - actormodel->getPosition( ) ) < closestVisibleDist ) {
					closestVisibleDist = Norm( actor.getPosition( ) - actormodel->getPosition( ) );
					closestVisibleActor = &actor;
				}
			} else {
				if( Norm( actor.getPosition( ) - actormodel->getPosition( ) ) < closestDist ) {
					closestDist = Norm( actor.getPosition( ) - actormodel->getPosition( ) );
					closestActor = &actor;
				}
			}
		}
	}

	return closestVisibleActor ? closestVisibleActor->getPosition( ) : closestActor ? closestActor->getPosition( ) : vec3f( std::numeric_limits< float >::infinity( ) );
}

int ActorControllerAI::findClosestVertex( const vec3f & position ) const {
	int closestvertex = -1;
	float closestdist = std::numeric_limits< float >::infinity( );
	for( int i = 0; i < pathgraph.numVertices( ); i++ ) {
		float dist = Norm( pathgraph.getVertexPosition( i ) - position );

		if( dist < closestdist ) {
			closestdist = dist;
			closestvertex = i;
		}
	}
	return closestvertex;
}

/*void ActorControllerAI::update( float time ) {
	int closestvertex = findClosestVertex( actormodel->getPosition( ) );

	{
		// calculate vertex neighbor energies
		const std::list< int > & neighbors = pathgraph.getNeighbors( closestvertex );

		int minengvertex = -1; //nextvertex;
		float mineng = calculateEnergy( neighbors.front( ) );

		for( std::list< int >::const_iterator iter = neighbors.begin( ); iter != neighbors.end( ); ++iter ) {
			float eng = calculateEnergy( *iter );

			if( eng < mineng ) {
				mineng = eng;
				minengvertex = *iter;
			}
		}
		
		//minengvertex = -1;

		if( minengvertex == -1 ) {
			std::vector< float > distances( pathgraph.numVertices( ) );
			std::vector< int > predecessors( pathgraph.numVertices( ) );
			pathgraph.shortestPath( closestvertex, distances, predecessors );

			const Actor * closestActor = 0;
			int closestActorVertex = -1;
			float closestPathlen = std::numeric_limits< float >::infinity( );
			for( std::list< Actor >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
				const Actor & actor = *iter;
				if( actormodel != &actor && actor.getState( ) == "idle" ) {
					int actorvertex = findClosestVertex( actor.getPosition( ) );
					if( distances[ actorvertex ] < closestPathlen ) {
						closestPathlen = distances[ actorvertex ];
						closestActor = &actor;
						closestActorVertex = actorvertex;
					}
				}
			}

			if( closestActor ) {
				int currvertex = closestActorVertex;
				while( currvertex > -1 && predecessors[ currvertex ] != closestvertex ) {
					currvertex = predecessors[ currvertex ];
				}
				nextvertex = currvertex > -1 ? currvertex : closestvertex;
			} else {
				nextvertex = closestvertex;
			}
		} else {
			nextvertex = minengvertex;
		}
	}

	greatestthreat = calculateThreat( );

	{
		float closestT;
		const PlanarVolume< 3, float, TexturedPlane< 3, float > > * collidedBrush;
		const TexturedPlane< 3, float > * collidedPlane;
		game->getClosestBrushCollision( actormodel->getPosition( ), greatestthreat - actormodel->getPosition( ), 0.0f, closestT, collidedBrush, collidedPlane );

		attacking = greatestthreat[ 0 ] < std::numeric_limits< float >::infinity( ) && closestT > 1.0f;
	}

	if( Norm( greatestthreat - actormodel->getPosition( ) ) < 64.0f ) {
		int i = 0;
		for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
			const Weapon & weapon = *iter;

			if( dynamic_cast< const ProjectileMelee * >( weapon.getModel( ) ) ) {
				weaponselection = i;
			}
		}
	} else if( Norm( greatestthreat - actormodel->getPosition( ) ) >= 64.0f && Norm( greatestthreat - actormodel->getPosition( ) ) < 256.0f ) {
		int i = 0;
		for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
			const Weapon & weapon = *iter;

			if( dynamic_cast< const ProjectileBullet * >( weapon.getModel( ) ) ) {
				weaponselection = i;
			}
		}
	} else if( Norm( greatestthreat - actormodel->getPosition( ) ) >= 256.0f ) {
		int i = 0;
		for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
			const Weapon & weapon = *iter;

			if( dynamic_cast< const ProjectileRocket * >( weapon.getModel( ) ) ) {
				weaponselection = i;
			}
		}
	}
}*/

void ActorControllerAI::update( float time ) {
	const static float actorweight = 1.0f;
	const static float itemweight = 1024.0f;

	int closestvertex = findClosestVertex( actormodel->getPosition( ) );
	if( closestvertex == -1 ) {
		//std::cout << actormodel->getName( ) << " " << actormodel->getState( ) << " ";
		//std::cout << actormodel->getPosition( )[ 0 ] << " " << actormodel->getPosition( )[ 1 ] << " " << actormodel->getPosition( )[ 2 ];
		//std::cout << " closestvertex == -1" << std::endl;
		return;
	}

	// determine move location
	// calculate vertex neighbor energies
	const std::list< int > & neighbors = pathgraph.getNeighbors( closestvertex );
	std::list< float > neighboreng( neighbors.size( ), 0.0f );

	{
		std::list< float >::iterator eiter = neighboreng.begin( );
		for( std::list< int >::const_iterator iter = neighbors.begin( ); iter != neighbors.end( ); ++iter, ++eiter ) {
			float eng = calculateEnergy( *iter );
			*eiter += eng;
		}
	}

	std::vector< float > distances( pathgraph.numVertices( ) );
	std::vector< int > predecessors( pathgraph.numVertices( ) );
	pathgraph.shortestPath( closestvertex, distances, predecessors );

	for( std::list< ActorModel >::const_iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); ++iter ) {
		const ActorModel & actor = *iter;
		//if( actormodel != &actor && actor.getState( ) == "idle" ) {
		if( actormodel != &actor && actor.getHealth( ) > 0.0f ) {
			int actorvertex = findClosestVertex( actor.getPosition( ) );
			if( actorvertex == -1 ) {
				//std::cout << actormodel->getName( ) << " " << actor.getName( ) << " actorvertex == -1" << std::endl;
				return;
			}

			float dist = distances[ actorvertex ];
			while( actorvertex > -1 && predecessors[ actorvertex ] != closestvertex ) {
				actorvertex = predecessors[ actorvertex ];
			}

			std::list< float >::iterator eiter = neighboreng.begin( );
			for( std::list< int >::const_iterator niter = neighbors.begin( ); niter != neighbors.end( ); ++niter, ++eiter ) {
				if( *niter == actorvertex ) {
					*eiter += actorweight * ( 1.0f / dist ) * -1.0f;
				}
			}
		}
	}

	for( std::list< Item * >::const_iterator iter = game->getItems( ).begin( ); iter != game->getItems( ).end( ); ++iter ) {
		const Item * item = *iter;

		int itemvertex = findClosestVertex( item->getPosition( ) );
		float dist = distances[ itemvertex ];
		while( itemvertex > -1 && predecessors[ itemvertex ] != closestvertex ) {
			itemvertex = predecessors[ itemvertex ];
		}

		std::list< float >::iterator eiter = neighboreng.begin( );
		for( std::list< int >::const_iterator niter = neighbors.begin( ); niter != neighbors.end( ); ++niter, ++eiter ) {
			if( *niter == itemvertex ) {
				*eiter += itemweight * ( 1.0f / dist ) * -1.0f;
			}
		}
	}

	int minvertex = -1;
	float minvertexeng = std::numeric_limits< float >::infinity( );
	{
		std::list< float >::iterator eiter = neighboreng.begin( );
		for( std::list< int >::const_iterator niter = neighbors.begin( ); niter != neighbors.end( ); ++niter, ++eiter ) {
			if( *eiter < minvertexeng ) {
				minvertexeng = *eiter;
				minvertex = *niter;
			}
		}
	}

	if( minvertex > -1 ) {
		nextvertex = minvertex;
	}

	// target an actor
	greatestthreat = calculateThreat( );

	{
		float closestT;
		const PlanarVolume< 3, float, TexturedPlane< 3, float > > * collidedBrush;
		const TexturedPlane< 3, float > * collidedPlane;
		game->getClosestBrushCollision( actormodel->getPosition( ), greatestthreat - actormodel->getPosition( ), 2.0f, closestT, collidedBrush, collidedPlane );

		attacking = greatestthreat[ 0 ] < std::numeric_limits< float >::infinity( ) && closestT > 1.0f;
	}

	if( attacking ) {
		if( Norm( greatestthreat - actormodel->getPosition( ) ) < 64.0f ) {
			int i = 0;
			for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
				const Weapon & weapon = *iter;

				if( dynamic_cast< const ProjectileMelee * >( weapon.getModel( ) ) ) {
					weaponselection = i;
				}
			}
		} else if( Norm( greatestthreat - actormodel->getPosition( ) ) >= 64.0f && Norm( greatestthreat - actormodel->getPosition( ) ) < 256.0f ) {
			int i = 0;
			for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
				const Weapon & weapon = *iter;

				if( dynamic_cast< const ProjectileBullet * >( weapon.getModel( ) ) ) {
					weaponselection = i;
				}
			}
		} else if( Norm( greatestthreat - actormodel->getPosition( ) ) >= 256.0f ) {
			int i = 0;
			for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
				const Weapon & weapon = *iter;

				if( dynamic_cast< const ProjectileRocket * >( weapon.getModel( ) ) ) {
					weaponselection = i;
				}
			}
		}
	} else {
		bool canbounce = false;
		float closestbounce = std::numeric_limits< float >::infinity( );
		vec3f closestbouncepoint;
		for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::const_iterator biter = game->getBrushes( ).begin( ); biter != game->getBrushes( ).end( ); ++biter ) {
			const PlanarVolume< 3, float, TexturedPlane< 3, float > > & brush = *biter;

			for( std::list< TexturedPlane< 3, float > >::const_iterator piter = brush.getPlanes( ).begin( ); piter != brush.getPlanes( ).end( ); ++piter ) {
				const TexturedPlane< 3, float > & plane = *piter;

				if( fabs( DotProduct( plane.GetNormal( ), Vector3( 0.0f, 1.0f, 0.0f ) ) ) < ( 1.0f / 32.0f ) ) {

					float h1 = plane.Distance( actormodel->getPosition( ) );
					float h2 = plane.Distance( greatestthreat );

					if( h1 > 0.0f && h2 > 0.0f ) {

						vec3f proj1 = actormodel->getPosition( ) - h1 * plane.GetNormal( );
						vec3f proj2 = greatestthreat - h2 * plane.GetNormal( );

						float frac = h1 / ( h1 + h2 );

						vec3f point = ( proj2 - proj1 ) * frac + proj1;

						if( brush.Within( point - plane.GetNormal( ) ) && ( h1 + h2 ) < closestbounce ) {
							float closestT1, closestT2;
							{
								const PlanarVolume< 3, float, TexturedPlane< 3, float > > * collidedBrush;
								const TexturedPlane< 3, float > * collidedPlane;
								game->getClosestBrushCollision( actormodel->getPosition( ), point + plane.GetNormal( ), 1.0f, closestT1, collidedBrush, collidedPlane );
							}
							{
								const PlanarVolume< 3, float, TexturedPlane< 3, float > > * collidedBrush;
								const TexturedPlane< 3, float > * collidedPlane;
								game->getClosestBrushCollision( point + plane.GetNormal( ), greatestthreat, 1.0f, closestT2, collidedBrush, collidedPlane );
							}

							if( closestT1 > 1.0f && closestT2 > 1.0f ) {
								closestbounce = h1 + h2;
								closestbouncepoint = point;
								canbounce = true;
							}
						}
					}
				}
			}
		}

		if( canbounce ) {
			int i = 0;
			for( std::list< Weapon >::const_iterator iter = actormodel->getWeapons( ).begin( ); iter != actormodel->getWeapons( ).end( ); ++iter, i++ ) {
				const Weapon & weapon = *iter;

				if( dynamic_cast< const ProjectileBounceLaser * >( weapon.getModel( ) ) ) {
					weaponselection = i;
					greatestthreat = closestbouncepoint;
					attacking = true;
					break;
				}
			}
		}
	}
}
