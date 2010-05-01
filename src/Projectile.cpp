#include "Projectile.h"
#include "GameModel.h"

Projectile::Projectile( float _mass, float _boundingRadius ) : owner( 0 ), mass( _mass ), boundingRadius( _boundingRadius ), velocity( 0.0f ), force( 0.0f ) { }

Projectile::~Projectile( ) { }

Projectile * Projectile::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	return 0;
}

const ActorModel * Projectile::getOwner( ) const { return owner; }

void Projectile::setOwner( const ActorModel * _owner ) { owner = _owner; }

bool Projectile::isDead( ) const { return true; }

bool Projectile::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	return false;
}

bool Projectile::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	return false;
}

bool Projectile::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	return false;
}

void Projectile::update( GameModel * game, float time ) {
}

// physics
vec3f & Projectile::getPosition( ) { return position; }
const vec3f & Projectile::getPosition( ) const { return position; }

vec3f & Projectile::getVelocity( ) { return velocity; }
const vec3f & Projectile::getVelocity( ) const { return velocity; }

vec3f & Projectile::getForce( ) { return force; }
const vec3f & Projectile::getForce( ) const { return force; }

float Projectile::getMass( ) const { return mass; }

float Projectile::getBoundingRadius( ) const { return boundingRadius; }

// extra
void Projectile::damageActor( GameModel * game, ActorModel & actor, float damage, float time ) const {
	//if( actor.getState( ) == "idle" && damage >= 1.0f ) {
	if( actor.getHealth( ) > 0.0f && damage >= 1.0f ) {
		actor.getHealth( ) -= damage;

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->actorDamaged( getOwner( ), actor, damage, time );
		}

		checkActorHealth( game, actor, time );
	}
}

void Projectile::checkActorHealth( GameModel * game, ActorModel & actor, float time ) const {
	//if( actor.getState( ) == "idle" && actor.getHealth( ) < 0.0f ) {
	if( actor.getHealth( ) <= 0.0f ) {
		//actor.getState( ) = "dying";
		//actor.getStateStart( ) = time;
		actor.getDeathTime( ) = time;
		//std::cout << &a << " killed by " << &getOwner( ) << std::endl;

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->actorKilled( getOwner( ), actor, time );
		}
	}
}

void Projectile::handleExplosion( GameModel * game, const Equation * forceEq, const Equation * damageEq, float time ) {
	// apply force and damage from explosion
	for( std::list< ActorModel >::iterator iter = game->getActors( ).begin( ); iter != game->getActors( ).end( ); iter++ ) {
		ActorModel & a = *iter;

		vec3f vec = a.getPosition( ) - getPosition( );

		if( a.getHealth( ) > 0.0f ) {
			vec3f newforce = vec * (*forceEq)( Norm( vec ) );
			a.getForce( ) += Vector3( newforce[ 0 ], 0.0f, newforce[ 2 ] );

			damageActor( game, a, (*damageEq)( Norm( vec ) ), time );
		}
	}
}


///////////////////////////////////////////////////////////

ProjectileMelee::ProjectileMelee( float _mass, float _boundingRadius, float _damage, float _alivetime ) : Projectile( _mass, _boundingRadius ), dead( false ), damage( _damage ), alivetime( _alivetime ) { }

Projectile * ProjectileMelee::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileMelee * proj = new ProjectileMelee( getMass( ), getBoundingRadius( ), damage, alivetime );
	proj->getPosition( ) = _position;
	proj->setOwner( _owner );
	proj->starttime = time;
	return proj;
}

bool ProjectileMelee::isDead( ) const {
	return dead;
}

bool ProjectileMelee::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
	}

	return false;
}

bool ProjectileMelee::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		dead = true;

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			//observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
		}
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileMelee::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
	}

	return false;
}

void ProjectileMelee::update( GameModel * game, float time ) {
	if( time - starttime > alivetime ) {
		dead = true;
	}
}

///////////////////////////////////////////////////////////

//ProjectileBullet::ProjectileBullet( float _mass, float _boundingRadius, float _damage ) { }
ProjectileBullet::ProjectileBullet( float _mass, float _boundingRadius, float _damage ) : Projectile( _mass, _boundingRadius ), dead( false ), damage( _damage ) { }

Projectile * ProjectileBullet::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileBullet * bullet = new ProjectileBullet( getMass( ), getBoundingRadius( ), damage );
	bullet->getPosition( ) = _position;
	bullet->setOwner( _owner );
	return bullet;
}

bool ProjectileBullet::isDead( ) const {
	return dead;
}

bool ProjectileBullet::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
	}

	return false;
}

bool ProjectileBullet::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		dead = true;

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
		}
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileBullet::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
	}

	return false;
}

void ProjectileBullet::update( GameModel * game, float time ) {
}

///////////////////////////////////////////////////////////

ProjectileBounceLaser::ProjectileBounceLaser( float _mass, float _boundingRadius, float _damage ) : Projectile( _mass, _boundingRadius ), dead( false ), damage( _damage ), bounces( 0 ) { }

Projectile * ProjectileBounceLaser::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileBounceLaser * proj = new ProjectileBounceLaser( getMass( ), getBoundingRadius( ), damage );
	proj->getPosition( ) = _position;
	proj->setOwner( _owner );
	proj->starttime = time;
	return proj;
}

bool ProjectileBounceLaser::isDead( ) const {
	return dead || bounces >= 3;
}

bool ProjectileBounceLaser::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
	}

	return false;
}

bool ProjectileBounceLaser::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		getVelocity( ) = getVelocity( ) - 2.0f * plane->GetNormal( ) * DotProduct( getVelocity( ), plane->GetNormal( ) );
		//dead = true;
		bounces++;

		if( bounces >= 3 ) {
			for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
				GameObserver * observer = *iter;
				observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
			}
		}
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileBounceLaser::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
	}

	return false;
}

void ProjectileBounceLaser::update( GameModel * game, float time ) {
	if( time - starttime > 10.0f ) {
		dead = true;
	}
}

float & ProjectileBounceLaser::getStartTime( ) {
	return starttime;
}

///////////////////////////////////////////////////////////

ProjectileRocket::ProjectileRocket( float _mass, float _boundingRadius, float _damage, const Equation * _forceEq, const Equation * _damageEq ) : Projectile( _mass, _boundingRadius ), damage( _damage ), forceEq( _forceEq ), damageEq( _damageEq ), dead( false ) { }

Projectile * ProjectileRocket::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileRocket * explosive = new ProjectileRocket( getMass( ), getBoundingRadius( ), damage, forceEq, damageEq );
	explosive->getPosition( ) = _position;
	explosive->setOwner( _owner );
	return explosive;
}

bool ProjectileRocket::isDead( ) const {
	return dead;
}

bool ProjectileRocket::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;

		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
		observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

bool ProjectileRocket::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		dead = true;

		handleExplosion( game, forceEq, damageEq, time );

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;

			observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
			observer->projectileExplosion( getPosition( ), time );
		}
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileRocket::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;

		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
		observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

void ProjectileRocket::update( GameModel * game, float time ) {
}

///////////////////////////////////////////////////////////

ProjectileGrenade::ProjectileGrenade( float _mass, float _boundingRadius, float _damage, float _alivetime, const Equation * _forceEq, const Equation * _damageEq ) : Projectile( _mass, _boundingRadius ), dead( false ), damage( _damage ), alivetime( _alivetime ), forceEq( _forceEq ), damageEq( _damageEq ) { }

Projectile * ProjectileGrenade::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileGrenade * proj = new ProjectileGrenade( getMass( ), getBoundingRadius( ), damage, alivetime, forceEq, damageEq );
	proj->getPosition( ) = _position;
	proj->setOwner( _owner );
	proj->starttime = time;
	return proj;
}

bool ProjectileGrenade::isDead( ) const {
	return dead;
}

bool ProjectileGrenade::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
		observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

bool ProjectileGrenade::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		getVelocity( ) = getVelocity( ) - 2.0f * plane->GetNormal( ) * DotProduct( getVelocity( ), plane->GetNormal( ) );
		getVelocity( ) *= 0.8f;
		//dead = true;

		/*for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
		}*/
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileGrenade::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
		observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

void ProjectileGrenade::update( GameModel * game, float time ) {
	getForce( ) += Vector3( 0.0f, -0.125f, 0.0f );

	if( time - starttime > alivetime ) {
		dead = true;

		handleExplosion( game, forceEq, damageEq, time );

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->projectileCollision( getPosition( ), vec3f( 0.0f ), time );
			observer->projectileExplosion( getPosition( ), time );
		}
	}
}


///////////////////////////////////////////////////////////

ProjectileFlame::ProjectileFlame( float _mass, float _boundingRadius, float _damage, float _alivetime ) : Projectile( _mass, _boundingRadius ), dead( false ), damage( _damage ), alivetime( _alivetime ) { }

Projectile * ProjectileFlame::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileFlame * proj = new ProjectileFlame( getMass( ), getBoundingRadius( ), damage, alivetime );
	proj->getPosition( ) = _position;
	proj->setOwner( _owner );
	proj->starttime = time;
	return proj;
}

bool ProjectileFlame::isDead( ) const {
	return dead;
}

bool ProjectileFlame::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );
	//actor.getHealth( ) -= damage;
	
	/*if( actor.getHealth( ) < 0.0f ) {
		actor.getState( ) = "dying";
		actor.getStateStart( ) = time;
		std::cout << &actor << " killed by " << &getOwner( ) << std::endl;
	}*/
	//checkActorHealth( game, actor, time );

	//getPosition( ) += getVelocity( ) * t;

	//dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		//observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
		//observer->projectileExplosion( getPosition( ), time );
	}

	return true;
}

bool ProjectileFlame::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	//getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		//getVelocity( ) = getVelocity( ) - 2.0f * plane->GetNormal( ) * DotProduct( getVelocity( ), plane->GetNormal( ) );
		//getVelocity( ) *= 0.8f;

		//vec3f reflection = getVelocity( ) - 2.0f * plane->GetNormal( ) * DotProduct( getVelocity( ), plane->GetNormal( ) );
		vec3f slide = getVelocity( ) - plane->GetNormal( ) * DotProduct( getVelocity( ), plane->GetNormal( ) );
		getVelocity( ) = slide;
		//dead = true;

		/*for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
		}*/
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileFlame::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;
		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
	}

	return false;
}

void ProjectileFlame::update( GameModel * game, float time ) {
	//getForce( ) += Vector3( 0.0f, -0.125f, 0.0f );

	if( time - starttime > alivetime ) {
		dead = true;

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;
			//observer->projectileCollision( getPosition( ), vec3f( 0.0f ), time );
			//observer->projectileExplosion( getPosition( ), time );
		}
	}
}

float ProjectileFlame::getStartTime( ) const {
	return starttime;
}

float & ProjectileFlame::getStartTime( ) {
	return starttime;
}

float ProjectileFlame::getAliveTime( ) const {
	return alivetime;
}

///////////////////////////////////////////////////////////

ProjectileBFG::ProjectileBFG( float _mass, float _boundingRadius, float _damage, int _spawnprojs ) : Projectile( _mass, _boundingRadius ), damage( _damage ), spawnprojs( _spawnprojs ), dead( false ) { }

Projectile * ProjectileBFG::clone( const vec3f & _position, const ActorModel * _owner, float time ) const {
	ProjectileBFG * proj = new ProjectileBFG( getMass( ), getBoundingRadius( ), damage, spawnprojs );
	proj->getPosition( ) = _position;
	proj->setOwner( _owner );
	return proj;
}

bool ProjectileBFG::isDead( ) const {
	return dead;
}

void ProjectileBFG::BFGExplode( GameModel * game, const vec3f & normal, float time ) {
	for( int i = 0; i < spawnprojs; i++ ) {
		vec3f randvec = Normalize( Vector3( uniformRand( ) - 0.5f, 0.0f, uniformRand( ) - 0.5f ) );
		if( DotProduct( randvec, normal ) < 0.0f ) {
			randvec = randvec - normal * DotProduct( randvec, normal ) * 2.0f;
		}

		if( spawnprojs > 1 ) {
			//ProjectileBFG * proj = new ProjectileBFG( getMass( ), getBoundingRadius( ), damage, spawnprojs / 8 );
			//ProjectileBullet * proj = new ProjectileBullet( 1.0f, 1.0f, 1.0f );
			{
				ProjectileBullet * proj = new ProjectileBullet( 1.0f, 1.0f, 2.0f );
				proj->getPosition( ) = getPosition( ) + normal * 16.0f;
				proj->setOwner( getOwner( ) );
				proj->getForce( ) = randvec * ( uniformRand( ) + 1.0f ) * 4.0f;
				game->getProjectiles( ).push_back( proj );
			}

			/*{
				ProjectileBounceLaser * proj = new ProjectileBounceLaser( 1.0f, 1.0f, 2.0f );
				proj->getPosition( ) = getPosition( ) + normal * 16.0f;
				proj->setOwner( &getOwner( ) );
				proj->getStartTime( ) = time;
				proj->getForce( ) = randvec * ( uniformRand( ) + 1.0f ) * 4.0f;
				game->getProjectiles( ).push_back( proj );
			}*/

			/*ProjectileRocket * proj = new ProjectileRocket( 1.0f, 1.0f, 1.0f );
			proj->getPosition( ) = getPosition( ) + normal * 1.0f;
			proj->setOwner( &getOwner( ) );
			proj->getStartTime( ) = time;
			proj->getForce( ) = randvec * ( uniformRand( ) + 1.0f ) * 4.0f;
			game->getProjectiles( ).push_back( proj );*/
		}
	}
}

bool ProjectileBFG::handleActorCollision( GameModel * game, ActorModel & actor, float t, float time ) {
	damageActor( game, actor, damage, time );

	getPosition( ) += getVelocity( ) * t;

	dead = true;

	BFGExplode( game, Normalize( getPosition( ) - actor.getPosition( ) ), time );
	//handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;

		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - actor.getPosition( ) ), time );
		//observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

bool ProjectileBFG::handleBrushCollision( GameModel * game, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	if( plane ) {
		dead = true;

		BFGExplode( game, plane->GetNormal( ), time );
		//handleExplosion( game, forceEq, damageEq, time );

		for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
			GameObserver * observer = *iter;

			observer->projectileCollision( getPosition( ), plane->GetNormal( ), time );
			//observer->projectileExplosion( getPosition( ), time );
		}
	} else {
		dead = true;
	}

	return false;
}

bool ProjectileBFG::handleProjectileCollision( GameModel * game, Projectile * projectile, float t, float time ) {
	getPosition( ) += getVelocity( ) * t;

	dead = true;

	//handleExplosion( game, forceEq, damageEq, time );

	for( std::list< GameObserver * >::iterator iter = game->getObservers( ).begin( ); iter != game->getObservers( ).end( ); iter++ ) {
		GameObserver * observer = *iter;

		observer->projectileCollision( getPosition( ), Normalize( getPosition( ) - projectile->getPosition( ) ), time );
		//observer->projectileExplosion( getPosition( ), time );
	}

	return false;
}

void ProjectileBFG::update( GameModel * game, float time ) {
}

