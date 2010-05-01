#include "GameModel.h"

float uniformRand( ) {
	return (float)rand( ) / RAND_MAX;
}

float sphereIntersection( const vec3f & position0, const vec3f & velocity0, float radius0, const vec3f & position1, const vec3f & velocity1, float radius1 ) {
	float a = NormSquared( velocity0 - velocity1 );
	float b = 2.0f * DotProduct( position0 - position1, velocity0 - velocity1 );
	float c = NormSquared( position0 - position1 ) - ( radius0 + radius1 ) * ( radius0 + radius1 );
	float discriminant = b * b - 4.0f * a * c;
	if( discriminant < 0.0f ) {
		return std::numeric_limits< float >::infinity( );
	} else {
		float t0 = ( -b - sqrt( discriminant ) ) / ( 2.0f * a );
		float t1 = ( -b + sqrt( discriminant ) ) / ( 2.0f * a );
		return t0;
	}
}

/*Brush::Brush( const PlanarVolume< 3, float, TexturedPlane< 3, float > > & _brush ) : brush( _brush ) {
}

template< class OffsetCalcT >
//PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > Brush::IntersectLine( const Matrix< 3, 1, float > & pointa, const Matrix< 3, 1, float > & pointb, const OffsetCalcT & offsetcalc ) const {
BrushIntersection Brush::IntersectLine( const Matrix< 3, 1, float > & pointa, const Matrix< 3, 1, float > & pointb, const OffsetCalcT & offsetcalc ) const {
	return BrushIntersection( &brush, brush.IntersectLine( pointa, pointb, offsetcalc ) );
}

float Brush::GreatestDistance( const kd::Plane< 3, float > & plane ) const {
	std::list< Matrix< 3, 1, float > > points = brush.GeneratePoints( );

	float max = -std::numeric_limits< float >::infinity( );
	for( std::list< Matrix< 3, 1, float > >::iterator iter = points.begin( ); iter != points.end( ); ++iter ) {
		if( plane.Distance( *iter ) > max ) {
			max = plane.Distance( *iter );
		}
	}

	return max;
}

float Brush::LeastDistance( const kd::Plane< 3, float > & plane ) const {
	std::list< Matrix< 3, 1, float > > points = brush.GeneratePoints( );

	float min = std::numeric_limits< float >::infinity( );
	for( std::list< Matrix< 3, 1, float > >::iterator iter = points.begin( ); iter != points.end( ); ++iter ) {
		if( plane.Distance( *iter ) < min ) {
			min = plane.Distance( *iter );
		}
	}

	return min;
}*/

Brush::Brush( const PlanarVolume< 3, float, TexturedPlane< 3, float > > & _brush ) : brush( _brush ) {
}

float Brush::GreatestDistance( const kd::Plane< 3, float > & plane ) const {
	//std::list< vec3f > points = brush.GeneratePoints( );
	std::list< vec3f > points = GeneratePoints( brush );

	float max = -std::numeric_limits< float >::infinity( );
	for( std::list< vec3f >::iterator iter = points.begin( ); iter != points.end( ); ++iter ) {
		if( plane.Distance( *iter ) > max ) {
			max = plane.Distance( *iter );
		}
	}

	return max;
}

float Brush::LeastDistance( const kd::Plane< 3, float > & plane ) const {
	//std::list< vec3f > points = brush.GeneratePoints( );
	std::list< vec3f > points = GeneratePoints( brush );

	float min = std::numeric_limits< float >::infinity( );
	for( std::list< vec3f >::iterator iter = points.begin( ); iter != points.end( ); ++iter ) {
		if( plane.Distance( *iter ) < min ) {
			min = plane.Distance( *iter );
		}
	}

	return min;
}

GameModel::~GameModel( ) {
	for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
		Projectile * & projectile = *iter;
		delete projectile;
	}

	for( std::list< Item * >::iterator iter = items.begin( ); iter != items.end( ); ++iter ) {
		Item * & item = *iter;
		delete item;
	}
}

void GameModel::initKdTree( ) {
	for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::iterator iter = brushes.begin( ); iter != brushes.end( ); ++iter ) {
		PlanarVolume< 3, float, TexturedPlane< 3, float > > & vol = *iter;

		kdTree.Add( Brush( vol ) );
	}

	kdTree.Build( 4 );
}

void GameModel::addSpawnPoint( const vec3f & point ) {
	spawnPoints.push_back( point );
}

/*void GameModel::addHealthSpawn( const vec3f & point ) {
	healthSpawns.push_back( HealthSpawn( point, -std::numeric_limits< float >::infinity( ) ) );
}*/

void GameModel::addWeaponSpawn( const vec3f & point, const Weapon & weapon ) {
	weaponSpawns.push_back( WeaponSpawn( point, weapon, -std::numeric_limits< float >::infinity( ) ) );
}

void GameModel::addPowerupSpawn( const vec3f & point, const std::string & type ) {
	powerupSpawns.push_back( PowerupSpawn( point, type, -std::numeric_limits< float >::infinity( ) ) );
}

void GameModel::addTrigger( const Trigger & trigger ) {
	triggers.push_back( trigger );
}

vec3f GameModel::getRandomSpawnPoint( ) const {
	int rindex;
	bool done;
	do {
		done = true;
		rindex = rand( ) % spawnPoints.size( );
		for( std::list< ActorModel >::const_iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			const ActorModel & actor = *iter;
			const Matrix< 2, 1, float > spawnpoint2d = Vector2( spawnPoints[ rindex ][ 0 ], spawnPoints[ rindex ][ 2 ] );
			const Matrix< 2, 1, float > position2d = Vector2( actor.getPosition( )[ 0 ], actor.getPosition( )[ 2 ] );
			if( Norm( spawnpoint2d - position2d ) < actor.getBoundingRadius( ) * 2.0f ) {
				done = false;
				break;
			}
		}
	} while( !done );

	return spawnPoints[ rindex ];
}

vec3f GameModel::getFurthestSpawnPoint( ) const {
	int furthest = -1;
	float furthestDist = -std::numeric_limits< float >::infinity( );

	for( size_t i = 0; i < spawnPoints.size( ); i++ ) {

		float closestActorDist = std::numeric_limits< float >::infinity( );

		for( std::list< ActorModel >::const_iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			const ActorModel & actor = *iter;

			//if( actor.getState( ) == "idle" ) {
			if( actor.getHealth( ) > 0.0f ) {
				if( Norm( spawnPoints[ i ] - actor.getPosition( ) ) < closestActorDist ) {
					closestActorDist = Norm( spawnPoints[ i ] - actor.getPosition( ) );
				}
			}
		}

		if( closestActorDist > furthestDist ) {
			furthestDist = closestActorDist;
			furthest = i;
		}
	}

	if( furthest == -1 ) {
		std::cout << "furthest == -1" << std::endl;
	}

	return spawnPoints[ furthest ];
}

// observers
void GameModel::addObserver( GameObserver * observer ) {
	observers.push_back( observer );
}

// collision stuff
void GameModel::getClosestActorCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, const ActorModel * actorIgnore, float & closestT, ActorModel * & collidedActor ) {
	collidedActor = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
		ActorModel & actor = *iter;

		//if( actorIgnore != &actor && ( actor.getState( ) != "dying" || actor.getHealth( ) > 0.0f ) ) {
		if( actorIgnore != &actor && actor.getHealth( ) >= 0.0f ) {
			float t = sphereIntersection( position, velocity, boundingRadius, actor.getPosition( ), actor.getVelocity( ), actor.getBoundingRadius( ) );
			
			if( ( t >= 0.0f && t < closestT ) || ( Norm( position - actor.getPosition( ) ) < boundingRadius + actor.getBoundingRadius( ) ) ) {
				closestT = t;
				collidedActor = &actor;
			}
		}
	}
}

// 0.007
/*void GameModel::getClosestBrushCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * & collidedBrush, const TexturedPlane< 3, float > * & collidedPlane ) const {
	collidedBrush = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::const_iterator iter = brushes.begin( ); iter != brushes.end( ); ++iter ) {
		const PlanarVolume< 3, float, TexturedPlane< 3, float > > & brush = *iter;

		//PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection = brush.IntersectLine( position, position + velocity, BoxOffsetCalc< 3, float >( Vector3( boundingRadius, boundingRadius, boundingRadius ) ) );
		PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection = brush.IntersectLine( position, position + velocity, SphereOffsetCalc< 3, float >( boundingRadius ) );
		
		if( intersection.t < closestT ) {
			closestT = intersection.t;
			collidedBrush = &brush;
			collidedPlane = intersection.plane;
		}
	}
}*/

// 0.0045
void GameModel::getClosestBrushCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * & collidedBrush, const TexturedPlane< 3, float > * & collidedPlane ) const {
	//static int calls = 0;
	//static float time = 0.0f;

	//float starttime = (float)glutGet( GLUT_ELAPSED_TIME );

	collidedBrush = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	BrushIntersection intersection = kdTree.IntersectLine( position, position + velocity, SphereOffsetCalc< 3, float >( boundingRadius ) );

	closestT = intersection.RayDistance( );
	collidedBrush = intersection.brush;
	collidedPlane = intersection.intersection.enterplane;

	//float endtime = (float)glutGet( GLUT_ELAPSED_TIME );

	//time += endtime - starttime;
	//calls++;

	//if( calls % 100 == 0 ) {
		//std::cout << ( time / calls ) << std::endl;
	//}
}

void GameModel::getClosestItemCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, Item **& collidedItem, std::map< Item *, bool > & itemsIgnore ) {
	collidedItem = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	for( std::list< Item * >::iterator iter = items.begin( ); iter != items.end( ); ++iter ) {
		Item * & item = *iter;

		if( item && ( itemsIgnore.find( item ) == itemsIgnore.end( ) || !itemsIgnore[ item ] ) ) {
			float t = sphereIntersection( position, velocity, boundingRadius, item->getPosition( ), vec3f( 0.0f ), itemBoundingRadius );
			
			if( ( t >= 0.0f && t < closestT ) || ( Norm( position - item->getPosition( ) ) < boundingRadius + itemBoundingRadius ) ) {
				closestT = t;
				collidedItem = &item;
			}
		}
	}
}

void GameModel::getClosestProjectileCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, const Projectile * projectileIgnore, float & closestT, Projectile * & collidedProjectile, std::map< Projectile *, bool > & projectilesIgnore ) {
	collidedProjectile = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
		Projectile * projectile = *iter;

		if( projectileIgnore != projectile && !projectile->isDead( ) && projectileIgnore && projectileIgnore->getOwner( ) != projectile->getOwner( ) && ( projectilesIgnore.find( projectile ) == projectilesIgnore.end( ) || !projectilesIgnore[ projectile ] ) ) {
			float t = sphereIntersection( position, velocity, boundingRadius, projectile->getPosition( ), projectile->getVelocity( ), projectile->getBoundingRadius( ) );
			
			if( ( t >= 0.0f && t < closestT ) || ( Norm( position - projectile->getPosition( ) ) < boundingRadius + projectile->getBoundingRadius( ) ) ) {
				closestT = t;
				collidedProjectile = projectile;
			}
		}
	}
}

void GameModel::getClosestTriggerCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, Trigger * & collidedTrigger, std::map< Trigger *, bool > & triggersIgnore ) {
	collidedTrigger = 0;
	closestT = std::numeric_limits< float >::infinity( );
	
	for( std::list< Trigger >::iterator iter = triggers.begin( ); iter != triggers.end( ); ++iter ) {
		Trigger & trigger = *iter;

		if( triggersIgnore.find( &trigger ) == triggersIgnore.end( ) || !triggersIgnore[ &trigger ] ) {
			std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & brushes = trigger.brushes;

			for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::iterator iter2 = brushes.begin( ); iter2 != brushes.end( ); ++iter2 ) {
				PlanarVolume< 3, float, TexturedPlane< 3, float > > & brush = *iter2;

				PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection = brush.IntersectLine( position, position + velocity, SphereOffsetCalc< 3, float >( boundingRadius ) );
				
				if( intersection.enterfraction > -std::numeric_limits< float >::infinity( ) && intersection.enterfraction < closestT || brush.Within( position ) ) {
					closestT = intersection.enterfraction;
					collidedTrigger = &(*iter);
				}
			}
		}
	}
}

void GameModel::handleActorActorCollision( ActorModel & actor1, ActorModel & actor2, float t ) {
	// correct position/velocity
	vec3f & pos1 = actor1.getPosition( );
	vec3f & pos2 = actor2.getPosition( );
	vec3f & vel1 = actor1.getVelocity( );
	vec3f & vel2 = actor2.getVelocity( );
	
	pos1 = pos1 + vel1 * ( t - 1.0f / 4096.0f );
	pos2 = pos2 + vel2 * ( t - 1.0f / 4096.0f );
	
	vec3f normal = Normalize( pos1 - pos2 );
	
	vel1 = vel1 - normal * DotProduct( vel1, normal );
	vel2 = vel2 - normal * DotProduct( vel2, normal );
}

void GameModel::handleActorBrushCollision( ActorModel & actor, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t ) {
	// correct position/velocity
	vec3f & pos = actor.getPosition( );
	vec3f & vel = actor.getVelocity( );

	if( plane ) {		
		pos = pos + vel * t;
		vel = vel - plane->GetNormal( ) * DotProduct( vel, plane->GetNormal( ) );
	} else {
		// ut oh
		pos = Vector3( 0.0f, 16.0f, 0.0f );
		vel = vec3f( 0.0f );
	}
}

bool GameModel::handleActorItemCollision( ActorModel & actor, Item * & item, float t, float time ) {
	if( item->activate( this, &actor ) ) {
		for( std::list< GameObserver * >::iterator iter = observers.begin( ); iter != observers.end( ); ++iter ) {
			(*iter)->itemPickedUp( actor, item, time );
		}

		delete item;
		item = 0; // mark for removal
		return true;
	} else {
		return false;
	}
}

bool GameModel::handleActorProjectileCollision( ActorModel & actor, Projectile * projectile, float t, float time ) {
	// correct position/velocity
	//actor.getPosition( ) += actor.getVelocity( ) * t;
	actor.getForce( ) += 0.5f * projectile->getVelocity( ) * projectile->getMass( );
	
	return projectile->handleActorCollision( this, actor, t, time );
}

void GameModel::handleActorTriggerCollision( ActorModel & actor, Trigger * trigger, float t, float time ) {
	std::cout << "trigger" << std::endl;
}

bool GameModel::handleBrushProjectileCollision( const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, Projectile * projectile, float t, float time ) {
	return projectile->handleBrushCollision( this, brush, plane, t, time );
}

bool GameModel::handleProjectileProjectileCollision( Projectile * projectile1, Projectile * projectile2, float t, float time ) {
	projectile1->handleProjectileCollision( this, projectile2, t, time );
	projectile2->handleProjectileCollision( this, projectile1, t, time );
	return false;
}

void GameModel::addActor( const ActorController * controller, const ActorModel & actor ) {
	for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
		if( iter->getName( ) == actor.getName( ) ) {
			// actor already exists
			return;
		}
	}

	actorcontrollers.push_back( controller );
	actors.push_back( actor );

	for( std::list< GameObserver * >::iterator iter = observers.begin( ); iter != observers.end( ); ++iter ) {
		GameObserver * observer = *iter;
		observer->addActor( actors.back( ) );
	}
}

void GameModel::removeActor( const std::string & name ) {
	std::list< const ActorController * >::iterator citer = actorcontrollers.begin( );
	for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter, ++citer ) {
		if( iter->getName( ) == name ) {
			for( std::list< Projectile * >::iterator piter = projectiles.begin( ); piter != projectiles.end( ); ++piter ) {
				Projectile * proj = *piter;

				if( proj->getOwner( ) == &(*iter) ) {
					proj->setOwner( 0 );
				}
			}

			for( std::list< GameObserver * >::iterator oiter = observers.begin( ); oiter != observers.end( ); ++oiter ) {
				GameObserver * observer = *oiter;
				observer->removeActor( *iter );
			}

			actorcontrollers.erase( citer );
			actors.erase( iter );
			break;
		}
	}
}

void GameModel::spawnActor( ActorModel & actor, float time ) {
	// reset physics
	actor.getPosition( ) = getFurthestSpawnPoint( );
	actor.getPosition( )[ 1 ] = actor.getBoundingRadius( );
	actor.getVelocity( ) = vec3f( 0.0f );
	actor.getForce( ) = vec3f( 0.0f );

	// reset attributes
	//actor.getState( ) = "idle";
	//actor.getStateStart( ) = time;
	actor.getHealth( ) = actorMaxHealth;

	// reset powerups
	actor.hasHomingPowerup( ) = false;
	actor.getAttackDelayModifier( ) = 1.0f;
	actor.getSpeedModifier( ) = 1.0f;
	actor.getPowerupLevel( ) = 0;
}

void GameModel::update( float time ) {
	//float starttime0 = (float)glutGet( GLUT_ELAPSED_TIME );

	// player respawn
	for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
		ActorModel & actor = *iter;

		//if( actor.getState( ) == "dying" && time - actor.getStateStart( ) > deathtime ) {
		if( actor.getHealth( ) <= 0.0f && time - actor.getDeathTime( ) > deathtime ) {
			spawnActor( actor, time );
		}
	}

	// weapon respawn
	for( std::list< WeaponSpawn >::iterator witer = weaponSpawns.begin( ); witer != weaponSpawns.end( ); ++witer ) {
		WeaponSpawn & weaponspawn = *witer;

		if( weaponspawn.t == std::numeric_limits< float >::infinity( ) ) {
			bool close = false;
			for( std::list< Item * >::iterator iiter = items.begin( ); iiter != items.end( ); ++iiter ) {
				Item * item = *iiter;

				if( Norm( item->getPosition( ) - weaponspawn.point ) < itemBoundingRadius * 2.0f ) {
					close = true;
					break;
				}
			}

			if( !close ) {
				weaponspawn.t = time;
			}
		} else {
			if( time - weaponspawn.t > itemRespawnTime ) {
				items.push_back( new ItemWeapon( weaponspawn.point, weaponspawn.weapon ) );
				weaponspawn.t = std::numeric_limits< float >::infinity( );
			}
		}
	}

	// powerup respawn
	for( std::list< PowerupSpawn >::iterator piter = powerupSpawns.begin( ); piter != powerupSpawns.end( ); ++piter ) {
		PowerupSpawn & powerupspawn = *piter;

		if( powerupspawn.t == std::numeric_limits< float >::infinity( ) ) {
			bool close = false;
			for( std::list< Item * >::iterator iiter = items.begin( ); iiter != items.end( ); ++iiter ) {
				Item * item = *iiter;

				if( Norm( item->getPosition( ) - powerupspawn.point ) < itemBoundingRadius * 2.0f ) {
					close = true;
					break;
				}
			}

			if( !close ) {
				powerupspawn.t = time;
			}
		} else {
			if( time - powerupspawn.t > itemRespawnTime ) {
				items.push_back( new ItemPowerup( powerupspawn.point, powerupspawn.type ) );
				powerupspawn.t = std::numeric_limits< float >::infinity( );
			}
		}
	}

	// handle actor input
	{
		{
			std::list< const ActorController * >::const_iterator citer = actorcontrollers.begin( );
			for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter, ++citer ) {
				ActorModel & actor = *iter;
				const ActorController * controller = *citer;

				//if( actor.getState( ) == "idle" ) {
				if( actor.getHealth( ) > 0.0f ) {
					
					// movement code
					vec3f movedir = controller->getMoveDirection( );
					actor.getForce( ) += movedir * actor.getSpeed( ) * actor.getSpeedModifier( );

					// look code
					vec3f lookdir = controller->getLookDirection( );
					actor.getOrientation( ) = lookdir;

					// weapon switching
					if( controller->getWeaponSwitch( ) != -1 ) {
						actor.switchWeapon( controller->getWeaponSwitch( ) );
					}

					// attacking code
					Weapon & currentWeapon = actor.getCurrentWeapon( );
					if( controller->isAttacking( ) && currentWeapon.isReady( time, actor.getAttackDelayModifier( ) ) ) {
						std::list< Projectile * > newproj = currentWeapon.createProjectile( actor.getPosition( ), time, &actor );
						for( std::list< Projectile * >::iterator iter = newproj.begin( ); iter != newproj.end( ); ++iter ) {
							(*iter)->getForce( ) += actor.getOrientation( ) * currentWeapon.getMuzzleSpeed( ) + currentWeapon.getSpread( ) * Vector3( uniformRand( ) - 0.5f, 0.0f * uniformRand( ) - 0.0f, uniformRand( ) - 0.5f );
							projectiles.push_back( *iter );
						}
						//projectiles.splice( projectiles.end( ), newproj );
						//projectiles.splice( projectiles.begin( ), newproj );

						for( std::list< GameObserver * >::iterator iter = observers.begin( ); iter != observers.end( ); ++iter ) {
							(*iter)->projectileCreation( actor, actor.getPosition( ), time );
						}
					}
				}
			}
		}

		// apply gravity, friction, etc,.
		{
			for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
				ActorModel & actor = *iter;

				if( actor.getHealth( ) > 0.0f ) {

					vec3f vel = actor.getVelocity( );
					if( Norm( vel ) > 0.0f ) {
						vec3f friction = actor.getMass( ) * 0.25f * 8.0f * Normalize( -vel );

						if( Norm( friction ) > Norm( vel ) ) {
							friction = Normalize( friction ) * Norm( vel );
						}

						actor.getForce( ) += friction;
					}

					// gravity
					//actor.getForce( ) += Vector3( 0.0f, -0.25f, 0.0f );
				}
			}
		}
	}

	// update projectiles
	for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
		Projectile * projectile = *iter;

		projectile->update( this, time );

		// homing projectiles
		/*float brushT = std::numeric_limits< float >::infinity( );
		for( std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > >::iterator iter = brushes.begin( ); iter != brushes.end( ); ++iter ) {
			PlanarVolume< 3, float, TexturedPlane< 3, float > > & brush = *iter;

			PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection = brush.IntersectRay( projectile->getOwner( ).getPosition( ), projectile->getOwner( ).getOrientation( ), ZeroOffsetCalc< 3, float >( ) );

			if( intersection.t < brushT ) {
				brushT = intersection.t;
			}
		}

		vec3f intersectionPos = projectile->getOwner( ).getPosition( ) + projectile->getOwner( ).getOrientation( ) * brushT;

		float speed = Norm( projectile->getVelocity( ) );
		projectile->getVelocity( ) += Normalize( intersectionPos - projectile->getPosition( ) );
		projectile->getVelocity( ) = speed * Normalize( projectile->getVelocity( ) );*/

		if( projectile->getOwner( ) && projectile->getOwner( )->hasHomingPowerup( ) ) {
			float closestAngle = 0.5f;
			ActorModel * closestActor = 0;

			for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
				ActorModel & actor = *iter;

				vec3f vec = actor.getPosition( ) - projectile->getPosition( );
				float angle = DotProduct( Normalize( vec ), Normalize( projectile->getVelocity( ) ) );
				//if( actor.getState( ) == "idle" && projectile->getOwner( ) != &actor && angle > closestAngle ) {
				if( actor.getHealth( ) > 0.0f && projectile->getOwner( ) != &actor && angle > closestAngle ) {
					closestAngle = angle;
					closestActor = &actor;
				}
			}

			if( closestActor ) {
				float speed = Norm( projectile->getVelocity( ) );
				projectile->getVelocity( ) += 4.0f * Normalize( closestActor->getPosition( ) - projectile->getPosition( ) );
				projectile->getVelocity( ) = speed * Normalize( projectile->getVelocity( ) );
			}
		}
	}

	// update physics
	{
		for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			ActorModel & actor = *iter;

			if( actor.getHealth( ) > 0.0f ) {
				
				// calculate velocity based on forces
				actor.getVelocity( ) += actor.getForce( ) * ( 1.0f / actor.getMass( ) );
				actor.getForce( ) = vec3f( 0.0f );
			}
		}
		
		for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
			Projectile * projectile = *iter;

			projectile->getVelocity( ) += projectile->getForce( ) * ( 1.0f / projectile->getMass( ) );
			projectile->getForce( ) = vec3f( 0.0f );
		}
	}

	// resolve all collisions
	std::map< Item *, bool > itemsIgnore;
	std::map< Projectile *, bool > projectilesIgnore;
	std::map< Trigger *, bool > triggersIgnore;
	int collisionCount = 0;
	int collisionsFound;
	do {
		collisionsFound = 0;

		for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			ActorModel & actor = *iter;

			if( actor.getHealth( ) > 0.0f ) {
				
				// get closest intersection
				float actorT;
				ActorModel * actorCollision;
				getClosestActorCollision( actor.getPosition( ), actor.getVelocity( ), actor.getBoundingRadius( ), &actor, actorT, actorCollision );
				
				float brushT;
				const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brushCollision;
				const TexturedPlane< 3, float > * planeCollision;
				getClosestBrushCollision( actor.getPosition( ), actor.getVelocity( ), actor.getBoundingRadius( ), brushT, brushCollision, planeCollision );

				float itemT;
				Item ** itemCollision;
				getClosestItemCollision( actor.getPosition( ), actor.getVelocity( ), actor.getBoundingRadius( ), itemT, itemCollision, itemsIgnore );

				float triggerT;
				Trigger * triggerCollision;
				getClosestTriggerCollision( actor.getPosition( ), actor.getVelocity( ), actor.getBoundingRadius( ), triggerT, triggerCollision, triggersIgnore );

				if( actorT < 1.0f || brushT < 1.0f || itemT < 1.0f || triggerT < 1.0f ) {
				//if( actorT <= 1.0f || brushT <= 1.0f ) {
					collisionsFound++;
					
					if( actorT < brushT && actorT < itemT && actorT < triggerT ) {
					//if( actorT < brushT ) {
						if( collisionCount > 20 ) {
							std::cout << "Actor-Actor Collision: " << actor.getName( ) << "-" << actorCollision->getName( ) << " " << actorT << std::endl;
						}
						handleActorActorCollision( actor, *actorCollision, actorT );
					} else if( brushT < actorT && brushT < itemT && brushT < triggerT ) {
					//} else {
						if( collisionCount > 20 ) {
							std::cout << "Actor-Brush Collision: " << actor.getName( ) << " " << brushT << std::endl;
						}
						handleActorBrushCollision( actor, brushCollision, planeCollision, brushT );
					} else if( itemT < actorT && itemT < brushT && itemT < triggerT ) {
						if( collisionCount > 20 ) {
							std::cout << "Actor-Item Collision: " << actor.getName( ) << " " << itemT << std::endl;
						}
						if( !handleActorItemCollision( actor, *itemCollision, itemT, time ) ) {
							// if the item was not picked up then ignore
							//collisionFound = false;

							// i'm pretty sure this shouldn't be decremented
							// because there could be an unresolved collision
							//collisionsFound--;
							itemsIgnore[ *itemCollision ] = true;
						}
					} else if( triggerT < actorT && triggerT < brushT && triggerT < itemT ) {
						if( collisionCount > 20 ) {
							std::cout << "Actor-Trigger Collision: " << actor.getName( ) << " " << triggerT << std::endl;
						}
						handleActorTriggerCollision( actor, triggerCollision, triggerT, time );
						triggersIgnore[ triggerCollision ] = true;
					}
				}
			}
		}

		for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
			Projectile * projectile = *iter;

			if( !projectile->isDead( ) && ( projectilesIgnore.find( projectile ) == projectilesIgnore.end( ) || !projectilesIgnore[ projectile ] ) ) {
				// get closest intersection
				float actorT;
				ActorModel * actorCollision;
				getClosestActorCollision( projectile->getPosition( ), projectile->getVelocity( ), projectile->getBoundingRadius( ), projectile->getOwner( ), actorT, actorCollision );
				
				float brushT;
				const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brushCollision;
				const TexturedPlane< 3, float > * planeCollision;
				getClosestBrushCollision( projectile->getPosition( ), projectile->getVelocity( ), projectile->getBoundingRadius( ), brushT, brushCollision, planeCollision );

				if( actorT <= 1.0f || brushT <= 1.0f ) {
					collisionsFound++;

					if( actorT < brushT ) {
						if( collisionCount > 20 ) {
							std::cout << "Actor-Projectile Collision: " << actorCollision->getName( ) << " " << actorT << std::endl;
						}
						if( handleActorProjectileCollision( *actorCollision, projectile, actorT, time ) ) {
							projectilesIgnore[ projectile ] = true;
							//collisionsFound--;
						}
					} else {
						if( collisionCount > 20 ) {
							std::cout << "Brush-Projectile Collision: " << brushT << std::endl;
						}
						if( handleBrushProjectileCollision( brushCollision, planeCollision, projectile, brushT, time ) ) {
							projectilesIgnore[ projectile ] = true;
							//collisionsFound--;
						}
					}
				}
			}
		}
		collisionCount++;
	//} while( collisionFound );
	} while( collisionsFound > 0 && collisionCount < 2 );

	// update living/dead status
	/*{
		for( std::list< Actor >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			Actor & actor = *iter;

			if( actor.getState( ) == "idle" && actor.getHealth( ) <= 0.0f ) {
				actor.getState( ) = "dying";
				actor.getStateStart( ) = time;
			}
		}
	}*/

	// update positions
	{
		for( std::list< ActorModel >::iterator iter = actors.begin( ); iter != actors.end( ); ++iter ) {
			ActorModel & actor = *iter;

			if( actor.getHealth( ) > 0.0f ) {
				actor.getPosition( ) += actor.getVelocity( );
			}
		}
		
		for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
			Projectile * projectile = *iter;
			projectile->getPosition( ) += projectile->getVelocity( );
		}
	}
	
	// remove dead projectiles
	for( std::list< Projectile * >::iterator iter = projectiles.begin( ); iter != projectiles.end( ); ++iter ) {
		Projectile * & projectile = *iter;

		if( projectile->isDead( ) ) {
			delete projectile;
			projectile = 0;
		}
	}
	projectiles.remove_if( std::bind2nd( std::equal_to< Projectile * >( ), (Projectile * )0 ) );
	//projectiles.remove_if( std::mem_fun( &Projectile::getDead ) );

	// remove activated items
	items.remove_if( std::bind2nd( std::equal_to< const Item * >( ), (Item * )0 ) );
}

std::list< const ActorController * > & GameModel::getActorControllers( ) {
	return actorcontrollers;
}

const std::list< const ActorController * > & GameModel::getActorControllers( ) const {
	return actorcontrollers;
}

std::list< ActorModel > & GameModel::getActors( ) {
	return actors;
}

const std::list< ActorModel > & GameModel::getActors( ) const {
	return actors;
}

std::list< Projectile * > & GameModel::getProjectiles( ) {
	return projectiles;
}

const std::list< Projectile * > & GameModel::getProjectiles( ) const {
	return projectiles;
}

std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & GameModel::getBrushes( ) {
	return brushes;
}

const std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & GameModel::getBrushes( ) const {
	return brushes;
}

std::list< Item * > & GameModel::getItems( ) {
	return items;
}

const std::list< Item * > & GameModel::getItems( ) const {
	return items;
}

std::list< GameObserver * > & GameModel::getObservers( ) {
	return observers;
}

const std::list< GameObserver * > & GameModel::getObservers( ) const {
	return observers;
}

void GameModel::setDeathTime( float _deathtime ) {
	deathtime = _deathtime;
}

void GameModel::setActorMaxHealth( float _actorMaxHealth ) {
	actorMaxHealth = _actorMaxHealth;
}

void GameModel::setItemRespawnTime( float _itemRespawnTime ) {
	itemRespawnTime = _itemRespawnTime;
}

void GameModel::setItemBoundingRadius( float _itemBoundingRadius ) {
	itemBoundingRadius = _itemBoundingRadius;
}

float GameModel::getItemBoundingRadius( ) const {
	return itemBoundingRadius;
}
