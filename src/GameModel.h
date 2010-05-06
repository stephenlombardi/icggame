/*****************************
Author: Steve Lombardi
Description: handles all game logic
*****************************/

#ifndef _GameModel_h
#define _GameModel_h

#include <cstdlib>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <slib/Matrix.hpp>
#include <slib/matrixutils.h>

#include <slib/kdTree.hpp>
#include <slib/PlanarVolume.hpp>
#include "TexturedPlane.h"

#include "GameObserver.h"
#include "ActorController.h"
#include "ActorModel.h"
#include "Weapon.h"
#include "Projectile.h"
//#include "Item.h"

//typedef Matrix< 3, 1, float > vec3f;

float uniformRand( );

float sphereIntersection( const vec3f & position0, const vec3f & velocity0, float radius0, const vec3f & position1, const vec3f & velocity1, float radius1 );

/*struct HealthSpawn {
	HealthSpawn( const vec3f & _point, float _t ) : point( _point ), t( _t ) { }
	vec3f point;
	float t;
};*/

struct WeaponSpawn {
	WeaponSpawn( const vec3f & _point, const Weapon & _weapon, float _t ) : point( _point ), weapon( _weapon ), t( _t ) { }
	vec3f point;
	Weapon weapon;
	float t;
};

struct PowerupSpawn {
	PowerupSpawn( const vec3f & _point, const std::string & _type, float _t ) : point( _point ), type( _type ), t( _t ) { }
	vec3f point;
	std::string type;
	float t;
};

struct Trigger {
	Trigger( const std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & _brushes ) : brushes( _brushes ) { }
	std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > brushes;
};

/*struct BrushIntersection {
	BrushIntersection( ) : brush( 0 ), intersection( -std::numeric_limits< float >::infinity( ), std::numeric_limits< float >::infinity( ), 0, 0 ) { }
	BrushIntersection( const PlanarVolume< 3, float, TexturedPlane< 3, float > > * _brush, PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > & _intersection ) : brush( _brush ), intersection( _intersection ) { }
	bool Intersection( ) const { return intersection.Intersection( ); }
	float RayDistance( ) const { return intersection.t; }
	const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush;
	PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection;
};

class Brush {
public:
	Brush( const PlanarVolume< 3, float, TexturedPlane< 3, float > > & _brush );

	template< class OffsetCalcT >
	BrushIntersection IntersectLine( const Matrix< 3, 1, float > & pointa, const Matrix< 3, 1, float > & pointb, const OffsetCalcT & offsetcalc ) const;
	float GreatestDistance( const kd::Plane< 3, float > & plane ) const;
	float LeastDistance( const kd::Plane< 3, float > & plane ) const;
private:
	PlanarVolume< 3, float, TexturedPlane< 3, float > > brush;
};*/

struct BrushIntersection {
	BrushIntersection( ) : brush( 0 ), intersection( PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > >( -std::numeric_limits< float >::infinity( ), std::numeric_limits< float >::infinity( ), 0, 0 ) ) {
	}

	BrushIntersection( const PlanarVolume< 3, float, TexturedPlane< 3, float > > * _brush, const PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > & _intersection ) : brush( _brush ), intersection( _intersection ) {
	}

	bool Intersection( ) const {
		return intersection.enterplane != 0;
	}

	float RayDistance( ) const {
		return intersection.enterplane ? intersection.enterfraction : std::numeric_limits< float >::infinity( );
	}

	const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush;
	PlanarVolumeIntersection< 3, float, TexturedPlane< 3, float > > intersection;
};

class Brush {
public:
	Brush( const PlanarVolume< 3, float, TexturedPlane< 3, float > > & _brush );

	template< class OffsetCalcT >
	BrushIntersection IntersectLine( const vec3f & pointa, const vec3f & pointb, const OffsetCalcT & offsetcalc ) const {
		return BrushIntersection( &brush, brush.IntersectLine( pointa, pointb, offsetcalc ) );
	}
	float GreatestDistance( const kd::Plane< 3, float > & plane ) const;
	float LeastDistance( const kd::Plane< 3, float > & plane ) const;
private:
	PlanarVolume< 3, float, TexturedPlane< 3, float > > brush;
};


class GameModel {
public:
	~GameModel( );

	void initKdTree( );

	// spawn points
	void addSpawnPoint( const vec3f & point );
	//void addHealthSpawn( const vec3f & point );
	void addWeaponSpawn( const vec3f & point, const Weapon & weapon );
	void addPowerupSpawn( const vec3f & point, const std::string & type );
	void addTrigger( const Trigger & trigger );

	vec3f getRandomSpawnPoint( ) const;
	vec3f getFurthestSpawnPoint( ) const;

	// observers
	void addObserver( GameObserver * observer );

	// collision stuff
	void getClosestActorCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, const ActorModel * actorIgnore, float & closestT, ActorModel * & collidedActor );
	void getClosestBrushCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * & collidedBrush, const TexturedPlane< 3, float > * & collidedPlane ) const;
	void getClosestItemCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, Item **& collidedItem, std::map< Item *, bool > & itemsIgnore );
	void getClosestProjectileCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, const Projectile * projectileIgnore, float & closestT, Projectile * & collidedProjectile, std::map< Projectile *, bool > & projectilesIgnore );
	void getClosestTriggerCollision( const vec3f & position, const vec3f & velocity, float boundingRadius, float & closestT, Trigger * & collidedTrigger, std::map< Trigger *, bool > & triggersIgnore );
	void handleActorActorCollision( ActorModel & actor1, ActorModel & actor2, float t );
	void handleActorBrushCollision( ActorModel & actor, const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, float t );
	// returns true if item has been picked up
	bool handleActorItemCollision( ActorModel & actor, Item * & item, float t, float time );
	// returns true if projectile should be ignored for the rest of the loop iteration
	bool handleActorProjectileCollision( ActorModel & actor, Projectile * projectile, float t, float time );
	void handleActorTriggerCollision( ActorModel & actor, Trigger * trigger, float t, float time );
	bool handleBrushProjectileCollision( const PlanarVolume< 3, float, TexturedPlane< 3, float > > * brush, const TexturedPlane< 3, float > * plane, Projectile * projectile, float t, float time );
	bool handleProjectileProjectileCollision( Projectile * projectile1, Projectile * projectile2, float t, float time );

	void addActor( const ActorController * controller, const ActorModel & actor );
	void removeActor( const std::string & name );
	void spawnActor( ActorModel & actor, float time );

	void update( float time );

	std::list< const ActorController * > & getActorControllers( );
	const std::list< const ActorController * > & getActorControllers( ) const;
	std::list< ActorModel > & getActors( );
	const std::list< ActorModel > & getActors( ) const;
	std::list< Projectile * > & getProjectiles( );
	const std::list< Projectile * > & getProjectiles( ) const;
	std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & getBrushes( );
	const std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > & getBrushes( ) const;
	std::list< Item * > & getItems( );
	const std::list< Item * > & getItems( ) const;
	std::list< GameObserver * > & getObservers( );
	const std::list< GameObserver * > & getObservers( ) const;

	void setDeathTime( float _deathtime );
	void setActorMaxHealth( float _actorMaxHealth );
	void setItemRespawnTime( float _itemRespawnTime );
	void setItemBoundingRadius( float _itemBoundingRadius );
	float getItemBoundingRadius( ) const;
private:	
	std::list< const ActorController * > actorcontrollers;
	std::list< ActorModel > actors;
	std::list< Projectile * > projectiles;
	std::list< PlanarVolume< 3, float, TexturedPlane< 3, float > > > brushes;
	std::list< Item * > items;
	std::list< Trigger > triggers;

	std::vector< vec3f > spawnPoints;
	//std::list< HealthSpawn > healthSpawns;
	std::list< WeaponSpawn > weaponSpawns;
	std::list< PowerupSpawn > powerupSpawns;

	std::list< GameObserver * > observers;

	kd::Tree< 3, BrushIntersection, Brush > kdTree;

	// globals
	float deathtime;
	float actorMaxHealth;
	float itemRespawnTime;
	float itemBoundingRadius;
};

#endif
