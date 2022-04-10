/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#pragma once
//SceneNode category used to dispatch commands
namespace Category
{
	enum Type
	{
		kNone = 0,
		kScene = 1 << 0,
		kPlatform = 1 << 1,
		kPlayerCharacter = 1 << 2,
		kEnemyCharacter = 1 << 3,
		kPickup = 1 << 4,
		kEnemyProjectile = 1 << 5,
		kAlliedProjectile = 1 << 6,
		kParticleSystem = 1 << 7,
		kSoundEffect = 1 << 8,
		kNetwork = 1 << 9,

		kProjectile = kAlliedProjectile | kEnemyProjectile,
		kCharacter = kPlayerCharacter | kEnemyCharacter
	};
}