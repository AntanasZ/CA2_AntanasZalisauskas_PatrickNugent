#pragma once
//SceneNode category used to dispatch commands
namespace Category
{
	enum Type
	{
		kNone = 0,
		kScene = 1 << 0,
		kPlatform = 1 << 1,
		kPlayerCharacter1 = 1 << 2,
		kPlayerCharacter2 = 1 << 3,
		kPlayerAircraft = 1 << 4,
		kAlliedAircraft = 1 << 5,
		kEnemyAircraft = 1 << 6,
		kEnemyCharacter = 1 << 7,
		kPickup = 1 << 8,
		kAlliedProjectile = 1 << 9,
		kEnemyProjectile = 1 << 10,
		kParticleSystem = 1 << 11,
		kSoundEffect = 1 << 12,
		kNetwork = 1 << 13,

		kAircraft = kPlayerAircraft | kAlliedAircraft | kEnemyAircraft,
		kProjectile = kAlliedProjectile | kEnemyProjectile,
		kCharacter = kPlayerCharacter1 | kPlayerCharacter2 | kEnemyCharacter
	};
}