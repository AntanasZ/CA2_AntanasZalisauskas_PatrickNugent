#include "DataTables.hpp"
#include "ParticleType.hpp"
#include "PickupType.hpp"
#include "PlatformType.hpp"
#include "ProjectileType.hpp"

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Data about each character type
///	Added jump height variables to characters
/// 
/// Edited by: Patrick Nugent
///
///	-Added comments to show which characters are players 
/// -Added enemy types
/// </summary>
///	<returns>Returns a vector of CharacterData which includes info about each character type</returns>
std::vector<CharacterData> InitializeCharacterData()
{
	std::vector<CharacterData> data(static_cast<int>(CharacterType::kCharacterCount));

	//Shaggy - playable character
	data[static_cast<int>(CharacterType::kShaggy)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kShaggy)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kShaggy)].m_texture = Textures::kShaggy;
	data[static_cast<int>(CharacterType::kShaggy)].m_jump_height = 200.f;
	data[static_cast<int>(CharacterType::kShaggy)].m_has_run_animation = true;

	//Scooby - playable character
	data[static_cast<int>(CharacterType::kScooby)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kScooby)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kScooby)].m_texture = Textures::kScooby;
	data[static_cast<int>(CharacterType::kScooby)].m_jump_height = 200.f;
	data[static_cast<int>(CharacterType::kScooby)].m_has_run_animation = true;

	//Fred - playable character
	data[static_cast<int>(CharacterType::kFred)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kFred)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kFred)].m_texture = Textures::kFred;
	data[static_cast<int>(CharacterType::kFred)].m_jump_height = 200.f;
	data[static_cast<int>(CharacterType::kFred)].m_has_run_animation = true;

	//Velma - playable character
	data[static_cast<int>(CharacterType::kVelma)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kVelma)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kVelma)].m_texture = Textures::kVelma;
	data[static_cast<int>(CharacterType::kVelma)].m_jump_height = 200.f;
	data[static_cast<int>(CharacterType::kVelma)].m_has_run_animation = true;

	//Daphne - playable character
	data[static_cast<int>(CharacterType::kDaphne)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kDaphne)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kDaphne)].m_texture = Textures::kDaphne;
	data[static_cast<int>(CharacterType::kDaphne)].m_jump_height = 200.f;
	data[static_cast<int>(CharacterType::kDaphne)].m_has_run_animation = true;

	//Creeper
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_speed = 80.f;
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_texture = Textures::kCreeper;
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kCreeperRight)] = data[static_cast<int>(CharacterType::kCreeperLeft)];
	data[static_cast<int>(CharacterType::kCreeperLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kCreeperRight)].m_directions.emplace_back(Direction(90.f, 1000.f));
	data[static_cast<int>(CharacterType::kCreeperRight)].m_has_run_animation = false;

	//Michael
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_texture = Textures::kMichael;
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kMichaelRight)] = data[static_cast<int>(CharacterType::kMichaelLeft)];
	data[static_cast<int>(CharacterType::kMichaelLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kMichaelRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Freddy
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_speed = 160.f;
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_texture = Textures::kFreddy;
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kFreddyRight)] = data[static_cast<int>(CharacterType::kFreddyLeft)];
	data[static_cast<int>(CharacterType::kFreddyLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kFreddyRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Jason
	data[static_cast<int>(CharacterType::kJasonLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kJasonLeft)].m_speed = 100.f;
	data[static_cast<int>(CharacterType::kJasonLeft)].m_texture = Textures::kJason;
	data[static_cast<int>(CharacterType::kJasonLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kJasonLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kJasonRight)] = data[static_cast<int>(CharacterType::kJasonLeft)];
	data[static_cast<int>(CharacterType::kJasonLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kJasonRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Stripe
	data[static_cast<int>(CharacterType::kStripeLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kStripeLeft)].m_speed = 270.f;
	data[static_cast<int>(CharacterType::kStripeLeft)].m_texture = Textures::kStripe;
	data[static_cast<int>(CharacterType::kStripeLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kStripeLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kStripeRight)] = data[static_cast<int>(CharacterType::kStripeLeft)];
	data[static_cast<int>(CharacterType::kStripeLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kStripeRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Garfield
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_speed = 230.f;
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_texture = Textures::kGarfield;
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kGarfieldRight)] = data[static_cast<int>(CharacterType::kGarfieldLeft)];
	data[static_cast<int>(CharacterType::kGarfieldLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kGarfieldRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Ghidorah
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_speed = 160.f;
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_texture = Textures::kGhidorah;
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kGhidorahRight)] = data[static_cast<int>(CharacterType::kGhidorahLeft)];
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-90.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-45.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-135.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-45.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-90.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-135.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-45.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahLeft)].m_directions.emplace_back(Direction(-90.f, 100.f));

	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(90.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(45.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(135.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(45.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(90.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(135.f, 100.f));
	data[static_cast<int>(CharacterType::kGhidorahRight)].m_directions.emplace_back(Direction(45.f, 100.f));

	//Pterodactyl
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_speed = 300.f;
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_texture = Textures::kPterodactyl;
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kPterodactylRight)] = data[static_cast<int>(CharacterType::kPterodactylLeft)];
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_directions.emplace_back(Direction(-135.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_directions.emplace_back(Direction(-90.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_directions.emplace_back(Direction(-45.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylLeft)].m_directions.emplace_back(Direction(-90.f, 200.f));

	data[static_cast<int>(CharacterType::kPterodactylRight)].m_directions.emplace_back(Direction(45.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylRight)].m_directions.emplace_back(Direction(90.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylRight)].m_directions.emplace_back(Direction(135.f, 200.f));
	data[static_cast<int>(CharacterType::kPterodactylRight)].m_directions.emplace_back(Direction(90.f, 200.f));

	//Turtle
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_speed = 80.f;
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_texture = Textures::kTurtle;
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kTurtleRight)] = data[static_cast<int>(CharacterType::kTurtleLeft)];
	data[static_cast<int>(CharacterType::kTurtleLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kTurtleRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Ghost
	data[static_cast<int>(CharacterType::kGhostLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kGhostLeft)].m_speed = 130.f;
	data[static_cast<int>(CharacterType::kGhostLeft)].m_texture = Textures::kGhost;
	data[static_cast<int>(CharacterType::kGhostLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kGhostLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kGhostRight)] = data[static_cast<int>(CharacterType::kGhostLeft)];
	data[static_cast<int>(CharacterType::kGhostLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kGhostRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Skull
	data[static_cast<int>(CharacterType::kSkullLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kSkullLeft)].m_speed = 200.f;
	data[static_cast<int>(CharacterType::kSkullLeft)].m_texture = Textures::kSkull;
	data[static_cast<int>(CharacterType::kSkullLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kSkullLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kSkullRight)] = data[static_cast<int>(CharacterType::kSkullLeft)];
	data[static_cast<int>(CharacterType::kSkullLeft)].m_directions.emplace_back(Direction(-90.f, 1000.f));
	data[static_cast<int>(CharacterType::kSkullRight)].m_directions.emplace_back(Direction(90.f, 1000.f));

	//Dutchman
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_hitpoints = 100;
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_speed = 250.f;
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_texture = Textures::kDutchman;
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_jump_height = 0.f;
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_has_run_animation = false;

	data[static_cast<int>(CharacterType::kDutchmanRight)] = data[static_cast<int>(CharacterType::kDutchmanLeft)];
	data[static_cast<int>(CharacterType::kDutchmanLeft)].m_directions.emplace_back(Direction(-65.f, 1000.f));
	data[static_cast<int>(CharacterType::kDutchmanRight)].m_directions.emplace_back(Direction(65.f, 1000.f));

	return data;
}

std::vector<ProjectileData> InitializeProjectileData()
{
	return std::vector<ProjectileData>();
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Data about each platform type
/// </summary>
/// <returns></returns>
std::vector<PlatformData> InitializePlatformData()
{
	std::vector<PlatformData> data(static_cast<int>(PlatformType::kPlatformCount));

	//Ground
	data[static_cast<int>(PlatformType::kGroundPlatform)].m_texture = Textures::kFloor;

	//Air Platforms
	data[static_cast<int>(PlatformType::kAirPlatform)].m_texture = Textures::kPlatform;

	return data;
}

std::vector<PickupData> InitializePickupData()
{
	std::vector<PickupData> data(static_cast<int>(PickupType::kPickupCount));

	const float pickupSpeed = 200.f;

	data[static_cast<int>(PickupType::kApple)].m_texture = Textures::kApple;
	data[static_cast<int>(PickupType::kApple)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kApple)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kOrange)].m_texture = Textures::kOrange;
	data[static_cast<int>(PickupType::kOrange)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kOrange)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kCake)].m_texture = Textures::kCake;
	data[static_cast<int>(PickupType::kCake)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kCake)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kCarrot)].m_texture = Textures::kCarrot;
	data[static_cast<int>(PickupType::kCarrot)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kCarrot)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kCookies)].m_texture = Textures::kCookies;
	data[static_cast<int>(PickupType::kCookies)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kCookies)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kDonut)].m_texture = Textures::kDonut;
	data[static_cast<int>(PickupType::kDonut)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kDonut)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kIceCream)].m_texture = Textures::kIceCream;
	data[static_cast<int>(PickupType::kIceCream)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kIceCream)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kMelon)].m_texture = Textures::kMelon;
	data[static_cast<int>(PickupType::kMelon)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kMelon)].m_directions.emplace_back(Direction(0.f, 1000.f));

	data[static_cast<int>(PickupType::kPancake)].m_texture = Textures::kPancake;
	data[static_cast<int>(PickupType::kPancake)].m_speed = pickupSpeed;
	data[static_cast<int>(PickupType::kPancake)].m_directions.emplace_back(Direction(0.f, 1000.f));
	return data;

	/*data[static_cast<int>(PickupType::kHealthRefill)].m_texture = Textures::kEntities;
	data[static_cast<int>(PickupType::kHealthRefill)].m_texture_rect = sf::IntRect(0, 64, 40, 40);
	data[static_cast<int>(PickupType::kHealthRefill)].m_action = [](Aircraft& a) {a.Repair(25); };

	data[static_cast<int>(PickupType::kMissileRefill)].m_texture = Textures::kEntities;
	data[static_cast<int>(PickupType::kMissileRefill)].m_texture_rect = sf::IntRect(40, 64, 40, 40);
	data[static_cast<int>(PickupType::kMissileRefill)].m_action = std::bind(&Aircraft::CollectMissiles, std::placeholders::_1, 3);

	data[static_cast<int>(PickupType::kFireSpread)].m_texture = Textures::kEntities;
	data[static_cast<int>(PickupType::kFireSpread)].m_texture_rect = sf::IntRect(80, 64, 40, 40);
	data[static_cast<int>(PickupType::kFireSpread)].m_action = std::bind(&Aircraft::IncreaseSpread, std::placeholders::_1);

	data[static_cast<int>(PickupType::kFireRate)].m_texture = Textures::kEntities;
	data[static_cast<int>(PickupType::kFireRate)].m_texture_rect = sf::IntRect(120, 64, 40, 40);
	data[static_cast<int>(PickupType::kFireRate)].m_action = std::bind(&Aircraft::IncreaseFireRate, std::placeholders::_1);
	return data;*/
}

std::vector<ParticleData> InitializeParticleData()
{
	std::vector<ParticleData> data(static_cast<int>(ParticleType::kParticleCount));

	data[static_cast<int>(ParticleType::kPropellant)].m_color = sf::Color(255, 255, 50);
	data[static_cast<int>(ParticleType::kPropellant)].m_lifetime = sf::seconds(0.6f);

	data[static_cast<int>(ParticleType::kSmoke)].m_color = sf::Color(50, 50, 50);
	data[static_cast<int>(ParticleType::kSmoke)].m_lifetime = sf::seconds(4.f);

	return data;
}



