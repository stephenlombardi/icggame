/*****************************
Author: Steve Lombardi
Description: Handles all rendering
*****************************/

#ifndef _GameView_h
#define _GameView_h

#include <GL/freeglut.h>
#include <GL/glext.h>
#include <slib/extensions.h>

#include <slib/Matrix.hpp>
#include <slib/PlanarVolume.hpp>
#include <slib/TGA.h>
#include <slib/drawableobject.h>
#include <slib/shader.h>
#include <slib/smf.h>
#include <slib/shapes.h>
#include <slib/OpenGLText.h>

#include <slib/MS3D.h>
#include "MS3DToMesh.h"
#include <slib/JointAnimation.h>
#include <slib/BoneMatrixContainer.h>

#include "GameObserver.h"
#include "GameModel.h"
#include "ActorModel.h"
#include "Projectile.h"
#include "Weapon.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include <boost/bind.hpp>

/*struct Light {
	Light( const vec3f & _position, const vec3f & _color, float _power ) : position( _position ), color( _color ), power( _power ) { }
	vec3f position;
	vec3f color;
	float power;
};*/

struct PointLight {
	PointLight( const vec3f & _position, const vec3f & _color, float _power, float _atten ) : position( _position ), color( _color ), power( _power ), atten( _atten ) { }
	vec3f position;
	vec3f color;
	float power;
	float atten;
};

struct SpotLight {
	SpotLight( const vec3f & _position, const vec3f & _direction, const vec3f & _color, float _fov, float _power, float _atten ) : position( _position ), direction( _direction ), color( _color ), fov( _fov ), power( _power ), atten( _atten ) { }
	vec3f position;
	vec3f direction;
	vec3f color;
	float fov;
	float power;
	float atten;
};

struct DirectionalLight {
	DirectionalLight( const vec3f & _direction, const vec3f & _color, float _power ) : direction( _direction ), color( _color ), power( _power ) { }
	vec3f direction;
	vec3f color;
	float power;
};

struct Particle {
	Particle( const vec3f & _position, const vec3f & _velocity, float _starttime, float _keepalive ) : position( _position ), velocity( _velocity ), starttime( _starttime ), keepalive( _keepalive ) { }
	bool isDead( float time ) { return time - starttime > keepalive; }
	vec3f position, velocity;
	float starttime, keepalive;
};

bool ProjCmp( const Projectile * proj1, const Projectile * proj2 );

mat4f KeyframeMatrix( const JointAnimation::JointPose & pose );

class GameView : public GameObserver {
public:
	GameView( );

	void init( );
	void initBrushTextures( );
	void uninit( );
	void uninitActorText( );
	void uninitBrushes( );
	void clearObjects( );

	void setGame( const GameModel * _game );
	void setCamera( const ActorModel * _camera );
	void setFirstPerson( );
	void setThirdPerson( );
	void setGlow( bool _doGlow );
	void setDistortion( bool _doDistortion );
	void setDrawProjectiles( bool _drawProjectiles );
	void setDrawParticles( bool _drawParticles );
	void setDynamicLighting( bool _doDynamicLighting );
	void setAmbientIntensity( float _ambientIntensity );

	//
	void setWalkAnimationSpeed( float _walkAnimationSpeed );

	void writeShadowMaps( const std::string & _shadowfilename );

	void getProjectionMatrix( float * projection ) const;
	void getViewMatrix( float * view ) const;
	vec3f getViewPos( ) const;
	vec3f getViewDir( ) const;

	// game observer
	void itemPickedUp( const ActorModel & actor, const Item * item, float starttime );
	void addActor( const ActorModel & actor );
	void removeActor( const ActorModel & actor );
	void actorSpawned( const ActorModel & actor, float starttime );
	void actorDamaged( const ActorModel * attacker, const ActorModel & attackee, float damage, float time );
	void actorKilled( const ActorModel * killer, const ActorModel & killee, float starttime );
	void projectileCreation( const ActorModel & actor, const vec3f & position, float starttime );
	void projectileCollision( const vec3f & position, const vec3f & normal, float starttime );
	void projectileExplosion( const vec3f & position, float starttime );

	void addLight( const vec3f & position, const vec3f & color, float power );
	void addPointLight( const PointLight & pointlight );
	void addSpotLight( const SpotLight & spotlight );
	void addDirectionalLight( const DirectionalLight & directionallight );

	void setBrushes( std::map< std::string, std::list< PlanarPolygon< 3, float, TexturedPlane< 3, float > > > > & texpolygons );
	void update( float time );
	void draw( float time );
private:
	void initFBO( );
	void initHUD( );
	void initShaders( );
	void initTextures( );
	void uninitTextures( );
	void initPlayerModel( );
	void initProjectileModels( );
	void initItemModels( );

	void updateActorText( const ActorModel & actor );
	
	void writeDepthMap( const std::string & suffix, int size, float exponent );
	void renderShadowMap( const float * lightview, const float * lightproj, const ActorModel * actorIgnore );
	void drawDeferredDirectionallight( const DirectionalLight & directionallight, const Matrix< 4, 4, float > & viewprojInv, int & lightiteration );
	void drawDeferredSpotlight( const SpotLight & spotlight, float * projection, float * view, const Matrix< 4, 4, float > & viewprojInv, const ActorModel * actorIgnore, int & lightiteration );
	void drawDeferredPointlight( const PointLight & pointlight, int & lightiteration );

	void drawActors( Shader & shader, const ActorModel * actorIgnore );
	void drawBrushes( Shader & shader, const std::string & texparamname, bool usetexture );
	void drawBrushesNormals( Shader & shader, const std::string & texparamname, bool usetexture );

	//void drawWorldShadowMapped( const float * projection, const float * view, const Light & light, const vec3f & lightdirection, const vec3f & lightup, float fov, float lightatten, int & lightiteration, const ActorModel * actorIgnore );
	//void drawWorldLit( const float * projection, const float * view, const Light & light, float lightatten, int & lightiteration );
	void drawHealthBar( const ActorModel & actor );
	void drawDyingActor( const ActorModel & actor, float time );
	void drawExplosionSphere( const Particle & explosion, float time );
	void drawItemEffect( const Particle & itemparticle, float time );


	void drawItemWeapon( const ItemWeapon * item, float time );
	void drawItemPowerup( const ItemPowerup * item, float time );

	void drawProjectileMelee( const ProjectileMelee * projectile, float time );
	void drawProjectileBullet( const ProjectileBullet * projectile, float time );
	void drawProjectileBounceLaser( const ProjectileBounceLaser * projectile, float time );
	void drawProjectileRocket( const ProjectileRocket * projectile, float time );
	void drawProjectileGrenade( const ProjectileGrenade * projectile, float time );
	void drawProjectileFlame( const ProjectileFlame * projectile, float time );
	void drawProjectileBFG( const ProjectileBFG * projectile, float time );

	static void projStretchVelocity( float * matrix, const Projectile * projectile );
	static void projIdentity( float * matrix, const Projectile * projectile );

	static void drawItemWeapon( GameView * gameview, const Item * item, float time );
	static void drawItemPowerup( GameView * gameview, const Item * item, float time );

	static void drawProjectileMelee( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileBullet( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileBounceLaser( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileRocket( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileGrenade( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileFlame( GameView * gameview, const Projectile * projectile, float time );
	static void drawProjectileBFG( GameView * gameview, const Projectile * projectile, float time );

	static void genTexture( std::istream & stream, GLuint & texid );
	static void genNormalTexture( std::istream & stream, GLuint & texid );

	Shader plainShader;
	Shader plainanimShader;
	Shader projShader, threedtexShader;
	DrawableObject playerModel, cubeShape, coneShape, sphereShape, weaponBox, hudquad, tessquad;
	static const int tesssize = 16;
	float playerModelSize;

	// animation
	BoneMatrixContainer boneMatrices;
	std::vector< JointAnimation > walkAnimation;
	std::map< std::string, float > walkTimer;
	float walkAnimationSpeed;

	GLuint notexTex;
	GLuint plusTex, powerupTex, homingTex, attackspeedTex, speedTex;
	GLuint knifeTex, machinegunTex, shotgunTex, rocketTex, grenadeTex, flameTex, bfgTex;
	GLuint lettersTex;
	GLuint actorTex;

	std::map< std::string, DrawableObject > brushPolygons;
	std::map< std::string, GLuint > textures;
	std::map< std::string, GLuint > normtextures;
	std::map< std::string, void (*)( GameView *, const Projectile *, float time ) > projDrawFuncs;
	std::map< std::string, void (*)( GameView *, const Item *, float time ) > itemDrawFuncs;
	std::map< std::string, GLuint > projTex;

	// graphics objects for scoreboard
	std::map< std::string, DrawableObject > actorNameText;
	std::map< std::string, int > actorKills;
	std::map< std::string, int > actorDeaths;
	std::map< std::string, float > actorDamageDealt;
	std::map< std::string, float > actorDamageReceived;
	std::map< std::string, DrawableObject > actorScoreText;

	std::map< const ActorModel *, float > muzzleFlares;

	const GameModel * game;
	const ActorModel * camera;
	float objdist; // max distances from objects of interest to the camera
	float cameraheight;
	bool firstperson;

	std::list< Particle > particles;
	std::list< Particle > bloodparticles;
	std::list< Particle > explosions;
	std::list< Particle > itemparticles;

	//std::list< Light > lights;
	std::list< PointLight > pointlights;
	std::list< SpotLight > spotlights;
	std::list< DirectionalLight > directionallights;

	GLuint threedfbo, ambientRT, diffuseRT, projRT, threedDepthRT;
	GLuint postprocfbo, postprocRT1, postprocRT2;
	GLuint shadowfbo, shadowDepthRT;
	int shadowfboSize;

	// deferred rendering
	GLuint gbufferfbo, galbedoRT, gnormalRT, gdepthRT;
	Shader gthreedShader, gthreedanimShader, gthreednormmapShader;
	Shader gpointlightShader, gspotlightShader, gdirlightShader, gambientShader;

	// rendering options
	bool doGlow;
	bool doDistortion;
	bool doDynamicLighting;
	bool drawProjectiles;
	bool drawParticles;
	float ambientIntensity;

	// debugging
	bool bWriteShadowMaps;
	std::string shadowfilename;
};

#endif