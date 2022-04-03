#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <limits>

#include "ParticleNode.hpp"
#include "ParticleType.hpp"
#include "Pickup.hpp"
#include "Platform.hpp"
#include "PlatformType.hpp"
#include "PostEffect.hpp"
#include "Projectile.hpp"
#include "SoundNode.hpp"
#include "Utility.hpp"

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph()
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, m_camera.getSize().y)//5000.f)
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.height - m_camera.getSize().y / 2.f)
	, m_scrollspeed(-50.f)
	, m_scrollspeed_compensation(1.f)
	, m_player_characters()
	//	, m_enemy_spawn_points()
	, m_active_enemies()
	, m_enemy_spawn_countdown()
	, m_pickup_spawn_countdown()
	, m_player_1_stun_countdown()
	, m_player_2_stun_countdown()
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
	, m_gravity(981.f)
	, m_game_countdown(sf::seconds(300))
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);

	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
}

void World::SetWorldScrollCompensation(float compensation)
{
	m_scrollspeed_compensation = compensation;
}

void World::Update(sf::Time dt)
{
	//Scroll the world
	//m_camera.move(0, m_scrollspeed * dt.asSeconds()*m_scrollspeed_compensation);
	//if (m_game_countdown > sf::Time::Zero)
	//{
		//Decrease and Display remaining game time
		//m_game_countdown -= dt;
		//DisplayRemainingGameTime();

		for (Character* a : m_player_characters)
		{
			a->SetVelocity(0.f, a->GetVelocity().y);
		}

		DestroyEntitiesOutsideView();
		//GuideMissiles();

		//Forward commands to the scenegraph until the command queue is empty
		while (!m_command_queue.IsEmpty())
		{
			m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
		}
		AdaptPlayerVelocity(dt);

		HandleCollisions();
		//Remove all destroyed entities
		//RemoveWrecks() only destroys the entities, not the pointers in m_player_aircraft
		auto first_to_remove = std::remove_if(m_player_characters.begin(), m_player_characters.end(), std::mem_fn(&Character::IsMarkedForRemoval));
		m_player_characters.erase(first_to_remove, m_player_characters.end());
		m_scenegraph.RemoveWrecks();

		//m_enemy_spawn_countdown += dt;
		//if (m_enemy_spawn_countdown >= sf::seconds(5.0f))
		//{
		//	SpawnEnemies();
		//	m_enemy_spawn_countdown = sf::seconds(0.f);
		//}

		////Spawn a flying enemy every 3 seconds and reset the spawn timer
		//m_flying_enemy_spawn_countdown += dt;
		//if (m_flying_enemy_spawn_countdown >= sf::seconds(3.0f))
		//{
		//	SpawnFlyingEnemies();
		//	m_flying_enemy_spawn_countdown = sf::seconds(0.f);
		//}

		////Spawn a pickup every 1.5 seconds and reset the spawn timer
		//m_pickup_spawn_countdown += dt;
		//if (m_pickup_spawn_countdown >= sf::seconds(1.5f))
		//{
		//	SpawnPickups();
		//	m_pickup_spawn_countdown = sf::seconds(0.f);
		//}

		for (Character* a : m_player_characters)
		{
			if(a->GetInvulnerable())
			{
				a->AddToStunTimer(dt);
				if (a->GetStunTimer() >= sf::seconds(3.0f))
				{
					a->SetStunned(false);
				}

				//enable collisions with enemies again after 5 seconds
				if (a->GetStunTimer() >= sf::seconds(5.0f))
				{
					a->SetInvulnerable(false);
					a->ResetStunTimer();
				}
			}
		}

		//Apply movement
		m_scenegraph.Update(dt, m_command_queue);
		AdaptPlayerPosition();

		UpdateSounds();
		UpdateCameraPosition();
	//}
	//else
	//{
	//	//End the game and wait 5 seconds while displaying the winning score
	//	m_game_countdown = sf::Time::Zero;
	//	m_gameover_countdown += dt;

	//	if (m_gameover_countdown >= sf::seconds(5.0f))
	//	{
	//		m_game_over = true;
	//	}
	//	else
	//	{
	//		//TODO - Rework because this causes game crash with how 2 players work now
	//		/*if (m_player_characters[0]->GetScore() > m_player_characters[1]->GetScore())
	//		{
	//			m_game_timer_display->SetString("Player 1 wins with " + std::to_string(m_player_characters[0]->GetScore()) + " points!");
	//		}
	//		else if (m_player_characters[1]->GetScore() > m_player_characters[0]->GetScore())
	//		{
	//			m_game_timer_display->SetString("Player 2 wins with " + std::to_string(m_player_characters[1]->GetScore()) + " points!");
	//		}
	//		else
	//		{
	//			m_game_timer_display->SetString("It's a draw, both players have " + std::to_string(m_player_characters[1]->GetScore()) + " points");
	//		}*/
	//	}
	//}
}

void World::Draw()
{
	if(PostEffect::IsSupported())
	{
		m_scene_texture.clear();
		m_scene_texture.setView(m_camera);
		m_scene_texture.draw(m_scenegraph);
		m_scene_texture.display();
		m_bloom_effect.Apply(m_scene_texture, m_target);
	}
	else
	{
		m_target.setView(m_camera);
		m_target.draw(m_scenegraph);
	}
}

Character* World::GetCharacter(int identifier) const
{
	for(Character * a : m_player_characters)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

void World::RemoveCharacter(int identifier)
{
	Character* character = GetCharacter(identifier);
	if (character)
	{
		character->Destroy();
		m_player_characters.erase(std::find(m_player_characters.begin(), m_player_characters.end(), character));
	}
}

/// <summary>
/// Edited by: Antanas Zalisaukas
///
///	Added a bool parameter and check to see if the character being added is a local player
///	and store their identifier if they are
/// </summary>
Character* World::AddCharacter(int identifier, CharacterType type, bool local_player)
{
	/*CharacterType player_character;
	if(m_player_characters.empty())
	{
		player_character = CharacterType::kShaggy;
	}
	else
	{
		player_character = CharacterType::kScooby;
	}*/

	if (local_player)
	{
		m_local_player_identifier = identifier;
	}

	std::unique_ptr<Character> player(new Character(type, m_textures, m_fonts));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);
	m_player_characters.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(player));

	

	return m_player_characters.back();
}

//void World::CreatePickup(sf::Vector2f position, PickupType type)
//{
//	std::unique_ptr<Pickup> pickup(new Pickup(type, m_textures));
//	pickup->setPosition(position);
//	pickup->SetVelocity(0.f, 1.f);
//	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(pickup));
//}

bool World::PollGameAction(GameActions::Action& out)
{
	return m_network_node->PollGameAction(out);
}

void World::SetCurrentBattleFieldPosition(float lineY)
{
	m_camera.setCenter(m_camera.getCenter().x, lineY - m_camera.getSize().y / 2);
	m_spawn_position.y = m_world_bounds.height;
}

void World::SetWorldHeight(float height)
{
	m_world_bounds.height = height;
}

bool World::HasAlivePlayer() const
{
	return !m_player_characters.empty();
}

bool World::HasPlayerReachedEnd() const
{
	if(Character* character = GetCharacter(1))
	{
		return !m_world_bounds.contains(character->getPosition());
	}
	return false;
}

void World::LoadTextures()
{
	m_textures.Load(Textures::kEntities, "Media/Textures/Entities.png");
	m_textures.Load(Textures::kJungle, "Media/Textures/Jungle.png");
	m_textures.Load(Textures::kExplosion, "Media/Textures/Explosion.png");
	m_textures.Load(Textures::kParticle, "Media/Textures/Particle.png");
	m_textures.Load(Textures::kFinishLine, "Media/Textures/FinishLine.png");

	m_textures.Load(Textures::kShaggy, "Media/Textures/ShaggyIdle.png");
	m_textures.Load(Textures::kShaggyStunned, "Media/Textures/ShaggyStunned.png");
	m_textures.Load(Textures::kShaggyRunning, "Media/Textures/ShaggyRunning.png");
	m_textures.Load(Textures::kScooby, "Media/Textures/ScoobyIdle.png");
	m_textures.Load(Textures::kScoobyStunned, "Media/Textures/ScoobyStunned.png");
	m_textures.Load(Textures::kScoobyRunning, "Media/Textures/ScoobyRunning.png");
	m_textures.Load(Textures::kFred, "Media/Textures/FredIdle.png");
	m_textures.Load(Textures::kFredStunned, "Media/Textures/FredStunned.png");
	m_textures.Load(Textures::kFredRunning, "Media/Textures/FredRunning.png");
	m_textures.Load(Textures::kVelma, "Media/Textures/VelmaIdle.png");
	m_textures.Load(Textures::kVelmaStunned, "Media/Textures/VelmaStunned.png");
	m_textures.Load(Textures::kVelmaRunning, "Media/Textures/velmaRunning.png");
	m_textures.Load(Textures::kDaphne, "Media/Textures/DaphneIdle.png");
	m_textures.Load(Textures::kDaphneStunned, "Media/Textures/DaphneStunned.png");
	m_textures.Load(Textures::kDaphneRunning, "Media/Textures/DaphneRunning.png");
	m_textures.Load(Textures::kMansion, "Media/Textures/Mansion.png");
	m_textures.Load(Textures::kCreeper, "Media/Textures/CreeperIdle.png");
	m_textures.Load(Textures::kMichael, "Media/Textures/MichaelIdle.png");
	m_textures.Load(Textures::kFloor, "Media/Textures/GroundPlatform.png");
	m_textures.Load(Textures::kPlatform, "Media/Textures/Platform.png");
	m_textures.Load(Textures::kFreddy, "Media/Textures/FreddyIdle.png");
	m_textures.Load(Textures::kJason, "Media/Textures/JasonIdle.png");
	m_textures.Load(Textures::kStripe, "Media/Textures/StripeIdle.png");
	m_textures.Load(Textures::kGarfield, "Media/Textures/GarfieldIdle.png");
	m_textures.Load(Textures::kGhidorah, "Media/Textures/GhidorahIdle.png");
	m_textures.Load(Textures::kPterodactyl, "Media/Textures/PterodactylIdle.png");
	m_textures.Load(Textures::kTurtle, "Media/Textures/TurtleIdle.png");
	m_textures.Load(Textures::kGhost, "Media/Textures/GhostIdle.png");
	m_textures.Load(Textures::kSkull, "Media/Textures/SkullIdle.png");
	m_textures.Load(Textures::kDutchman, "Media/Textures/DutchmanIdle.png");

	m_textures.Load(Textures::kApple, "Media/Textures/Apple.png");
	m_textures.Load(Textures::kOrange, "Media/Textures/Orange.png");
	m_textures.Load(Textures::kCake, "Media/Textures/Cake.png");
	m_textures.Load(Textures::kCarrot, "Media/Textures/Carrot.png");
	m_textures.Load(Textures::kCookies, "Media/Textures/Cookies.png");
	m_textures.Load(Textures::kDonut, "Media/Textures/Donut.png");
	m_textures.Load(Textures::kIceCream, "Media/Textures/IceCream.png");
	m_textures.Load(Textures::kMelon, "Media/Textures/Melon.png");
	m_textures.Load(Textures::kPancake, "Media/Textures/Pancake.png");
}

void World::BuildScene()
{
	//Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(Layers::kLayerCount); ++i)
	{
		Category::Type category = (i == static_cast<int>(Layers::kLowerAir)) ? Category::Type::kScene : Category::Type::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(Textures::kMansion);
	//sf::IntRect textureRect(m_world_bounds);
	//Tile the texture to cover our world
	texture.setRepeated(true);

	//float view_height = m_camera.getSize().y;
	sf::IntRect texture_rect(m_world_bounds);
	//texture_rect.height += static_cast<int>(view_height);

	//Add the background sprite to our scene
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, texture_rect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top);
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(background_sprite));

	// Add the finish line to the scene
	/*sf::Texture& finish_texture = m_textures.Get(Textures::kFinishLine);
	std::unique_ptr<SpriteNode> finish_sprite(new SpriteNode(finish_texture));
	finish_sprite->setPosition(0.f, -76.f);
	m_finish_sprite = finish_sprite.get();
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(finish_sprite));*/

	// Add particle node to the scene
	//std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	//m_scene_layers[static_cast<int>(Layers::kLowerAir)]->AttachChild(std::move(smokeNode));

	//// Add propellant particle node to the scene
	//std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	//m_scene_layers[static_cast<int>(Layers::kLowerAir)]->AttachChild(std::move(propellantNode));

	//Prepare platforms
	std::unique_ptr<Platform> ground_platform(new Platform(PlatformType::kGroundPlatform, m_textures));
	ground_platform->setPosition(m_world_bounds.width / 2, 755.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(ground_platform));

	std::unique_ptr<Platform> platform1(new Platform(PlatformType::kAirPlatform, m_textures));
	platform1->setPosition(525.f, 450.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platform1));

	std::unique_ptr<Platform> platform2(new Platform(PlatformType::kAirPlatform, m_textures));
	platform2->setPosition(800.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platform2));

	std::unique_ptr<Platform> platform3(new Platform(PlatformType::kAirPlatform, m_textures));
	platform3->setPosition(250.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platform3));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	//Add game timer
	std::unique_ptr<TextNode> gameTimerDisplay(new TextNode(m_fonts, ""));
	gameTimerDisplay->setPosition(m_world_bounds.width / 2, 20);
	gameTimerDisplay->SetColor(sf::Color::Yellow);
	m_game_timer_display = gameTimerDisplay.get();
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(gameTimerDisplay));

	if(m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode());
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}

	srand(time(NULL));

	AddEnemies();
	AddPickups();
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

void World::AdaptPlayerPosition()
{
	//Keep all players on the screen, at least border_distance from the border
	sf::FloatRect view_bounds = GetViewBounds();
	const float border_distance = 48.f;

	for (Character* character : m_player_characters)
	{
		sf::Vector2f position = character->getPosition();
		position.x = std::max(position.x, view_bounds.left + border_distance);
		position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
		position.y = std::max(position.y, view_bounds.top + border_distance);
		position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance - 10.f);
		character->setPosition(position);
	}
}

void World::AdaptPlayerVelocity(sf::Time dt)
{
	for (Character* character : m_player_characters)
	{
		//sf::Vector2f velocity = aircraft->GetVelocity();
		////if moving diagonally then reduce velocity
		//if (velocity.x != 0.f && velocity.y != 0.f)
		//{
		//	aircraft->SetVelocity(velocity / std::sqrt(2.f));
		//}
		//Add gravity velocity
		character->Accelerate(0.f, m_gravity * dt.asSeconds());
	}
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn offscreen
	sf::FloatRect bounds = GetViewBounds();
	bounds.left -= 100.f;
	bounds.width += 100.f;

	return bounds;
}

void World::SpawnEnemies(sf::Int8 enemyType)
{
	//Spawn a random enemy from the vector of enemy spawn points
	CharacterSpawnPoint spawn = m_enemy_spawn_points[enemyType];
	std::unique_ptr<Character> enemy(new Character(spawn.m_type, m_textures, m_fonts));
	enemy->setPosition(spawn.m_x, spawn.m_y);

	//If an enemy is spawning on the right side then flip the sprite
	if (spawn.m_x > 100)
	{
		enemy->FlipSprite();
	}
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(enemy));
}

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Same as SpawnEnemies but handles flying enemies instead
/// -Made changes so that the random values are passed in
/// </summary>
void World::SpawnFlyingEnemies(sf::Int8 enemyType)
{
	//Spawn a random flying enemy from the vector of flying enemy spawn points
	CharacterSpawnPoint spawn = m_flying_enemy_spawn_points[enemyType];
	std::unique_ptr<Character> enemy(new Character(spawn.m_type, m_textures, m_fonts));
	enemy->setPosition(spawn.m_x, spawn.m_y);

	//If an enemy is spawning on the right side then flip the sprite
	if (spawn.m_x > 100)
	{
		enemy->FlipSprite();
	}
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(enemy));
}

/// <summary>
/// created By: Patrick Nugent
///
///	-Works similar to SpawnEnemies but modified to use pickups
///
/// Edited By: Patrick Nugent
///
/// -Added pickup values
/// -Made changes so that the random values are passed in
/// </summary>
void World::SpawnPickups(sf::Int8 pickupType, sf::Int16 pickupPosition)
{
	if(pickupType >= 0 && pickupType < m_pickup_spawn_points.size())
	{
		//Spawn a random pickup from the vector of pickup spawn points
		PickupSpawnPoint spawn = m_pickup_spawn_points[pickupType];
		std::unique_ptr<Pickup> pickup(new Pickup(spawn.m_type, spawn.m_value, m_textures));

		//Use the random x value for the pickup's position (within the bounds)
		pickup->setPosition((float)pickupPosition, spawn.m_y);

		m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(pickup));
	}
}

void World::AddEnemy(CharacterType type, bool isFlying, float relX, float relY)
{
	/*SpawnPoint spawn(type, m_spawn_position.x + relX, m_spawn_position.y - relY);
	m_enemy_spawn_points.emplace_back(spawn);*/

	CharacterSpawnPoint spawn(type, m_spawn_position.x + relX, m_spawn_position.y - relY);
	if (isFlying)
	{
		m_flying_enemy_spawn_points.emplace_back(spawn);
	}
	else
	{
		m_enemy_spawn_points.emplace_back(spawn);
	}
}

/// <summary>
/// Created by: Patrick Nugent
///
///	-Works similar to AddEnemy but uses the PickupSpawnPoint struct instead
///
/// Edited By: Patrick Nugent
///
/// -Added pickup values
/// </summary>
void World::AddPickup(PickupType type, int value, float relX, float relY)
{
	PickupSpawnPoint spawn(type, value, m_spawn_position.x + relX, m_spawn_position.y - relY);
	m_pickup_spawn_points.emplace_back(spawn);
}

void World::AddEnemies()
{
	/*if(m_networked_world)
	{
		return;
	}*/

	//Add all enemies - both the left and right side versions
	AddEnemy(CharacterType::kCreeperLeft, false, -500.f, -329.5f);
	AddEnemy(CharacterType::kCreeperRight, false, 500.f, -329.5f);
	AddEnemy(CharacterType::kMichaelLeft, false, -500.f, -325.f);
	AddEnemy(CharacterType::kMichaelRight, false, 500.f, -325.f);
	AddEnemy(CharacterType::kFreddyLeft, false, -500.f, -323.f);
	AddEnemy(CharacterType::kFreddyRight, false, 500.f, -323.f);
	AddEnemy(CharacterType::kJasonLeft, false, -500.f, -321.f);
	AddEnemy(CharacterType::kJasonRight, false, 500.f, -321.f);
	AddEnemy(CharacterType::kStripeLeft, false, -500.f, -335.f);
	AddEnemy(CharacterType::kStripeRight, false, 500.f, -335.f);
	AddEnemy(CharacterType::kGarfieldLeft, false, -500.f, -331.f);
	AddEnemy(CharacterType::kGarfieldRight, false, 500.f, -331.f);

	AddEnemy(CharacterType::kGhidorahLeft, true, -500.f, -2.f);
	AddEnemy(CharacterType::kGhidorahRight, true, 500.f, -2.f);
	AddEnemy(CharacterType::kPterodactylLeft, true, -500.f, -2.f);
	AddEnemy(CharacterType::kPterodactylRight, true, 500.f, -2.f);
	AddEnemy(CharacterType::kTurtleLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kTurtleRight, true, 500.f, -150.f);
	AddEnemy(CharacterType::kGhostLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kGhostRight, true, 500.f, -150.f);
	AddEnemy(CharacterType::kSkullLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kSkullRight, true, 500.f, -150.f);
	AddEnemy(CharacterType::kDutchmanLeft, true, -500.f, 200.f);
	AddEnemy(CharacterType::kDutchmanRight, true, 500.f, 200.f);

	//Add all enemies
	/*AddEnemy(::kRaptor, 0.f, 500.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 1000.f);
	AddEnemy(AircraftType::kRaptor, +100.f, 1150.f);
	AddEnemy(AircraftType::kRaptor, -100.f, 1150.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1710.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1700.f);
	AddEnemy(AircraftType::kAvenger, 30.f, 1850.f);
	AddEnemy(AircraftType::kRaptor, 300.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, -300.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 2500.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 3000.f);
	AddEnemy(AircraftType::kRaptor, 250.f, 3250.f);
	AddEnemy(AircraftType::kRaptor, -250.f, 3250.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3500.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3700.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 3800.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 4000.f);
	AddEnemy(AircraftType::kAvenger, -200.f, 4200.f);
	AddEnemy(AircraftType::kRaptor, 200.f, 4200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 4400.f);*/

	//Sort according to y value so that lower enemies are checked first
	//SortEnemies();
}

//void World::SortEnemies()
//{
//	//Sort all enemies according to their y-value, such that lower enemies are checked first for spawning
//	std::sort(m_enemy_spawn_points.begin(), m_enemy_spawn_points.end(), [](SpawnPoint lhs, SpawnPoint rhs)
//	{
//		return lhs.m_y < rhs.m_y;
//	});
//}

/// <summary>
/// Created By: Patrick Nugent
///
///	-Works similar to AddEnemies but modified to use pickups
///
/// Edited By: Patrick Nugent
///
/// -Added pickup values
/// </summary>
void World::AddPickups()
{
	//400
	float yPosition = 400.f;

	//Add all types of pickups and set their score values
	AddPickup(PickupType::kApple, 10, 0.f, yPosition);
	AddPickup(PickupType::kOrange, 10, 0.f, yPosition);
	AddPickup(PickupType::kCake, 50, 0.f, yPosition);
	AddPickup(PickupType::kCarrot, 20, 0.f, yPosition);
	AddPickup(PickupType::kCookies, 25, 0.f, yPosition);
	AddPickup(PickupType::kDonut, 30, 0.f, yPosition);
	AddPickup(PickupType::kIceCream, 40, 0.f, yPosition);
	AddPickup(PickupType::kMelon, 35, 0.f, yPosition);
	AddPickup(PickupType::kPancake, 30, 0.f, yPosition);
}

/*void World::GuideMissiles()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = Category::kEnemyAircraft;
	enemyCollector.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
	{
		if (!enemy.IsDestroyed())
			m_active_enemies.emplace_back(&enemy);
	});

	// Setup command that guides all missiles to the enemy which is currently closest to the player
	Command missileGuider;
	missileGuider.category = Category::kAlliedProjectile;
	missileGuider.action = DerivedAction<Projectile>([this](Projectile& missile, sf::Time)
	{
		// Ignore unguided bullets
		if (!missile.IsGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Character* closestEnemy = nullptr;

		// Find closest enemy
		for(Character * enemy :  m_active_enemies)
		{
			float enemyDistance = Distance(missile, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			missile.GuideTowards(closestEnemy->GetWorldPosition());
	});

	// Push commands, reset active enemies
	m_command_queue.Push(enemyCollector);
	m_command_queue.Push(missileGuider);
	m_active_enemies.clear();
}*/

bool MatchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();
	if(type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if(type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::HandleCollisions()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for(SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, Category::Type::kPlatform, Category::Type::kPlayerCharacter1))
		{
			auto& platform = static_cast<Platform&>(*pair.first);
			auto& player = static_cast<Character&>(*pair.second);

			if (player.GetWorldPosition().y < platform.GetWorldPosition().y)
			{
				player.ToggleCanJump(true);
				player.move(0.f, -1.f);
				player.SetVelocity(player.GetVelocity().x, 0);
			}
			else if (player.GetWorldPosition().y > platform.GetWorldPosition().y)
			{
				player.move(0.f, 1.f);
				player.SetVelocity(player.GetVelocity().x, 0);
			}
		}
		else if (MatchesCategories(pair, Category::Type::kPlatform, Category::Type::kPlayerCharacter2))
		{
			auto& platform = static_cast<Platform&>(*pair.first);
			auto& player = static_cast<Character&>(*pair.second);

			if (player.GetWorldPosition().y < platform.GetWorldPosition().y)
			{
				player.ToggleCanJump(true);
				player.move(0.f, -1.f);
				player.SetVelocity(player.GetVelocity().x, 0);
			}
			else if (player.GetWorldPosition().y > platform.GetWorldPosition().y)
			{
				player.move(0.f, 1.f);
				player.SetVelocity(player.GetVelocity().x, 0);
			}
		}
		else if (MatchesCategories(pair, Category::Type::kPlayerCharacter1, Category::Type::kEnemyCharacter) || MatchesCategories(pair, Category::Type::kPlayerCharacter2, Category::Type::kEnemyCharacter))
		{
			auto& player = static_cast<Character&>(*pair.first);
			if (!player.GetInvulnerable())
			{
				m_sounds.Play(SoundEffect::kStun);
				player.SetStunned(true);
				player.SetInvulnerable(true);
			}
		}
		else if (MatchesCategories(pair, Category::Type::kPlayerCharacter1, Category::Type::kPickup) || MatchesCategories(pair, Category::Type::kPlayerCharacter2, Category::Type::kPickup))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			m_sounds.Play(SoundEffect::kCollectPickup);

			//Add the pickup's value to the player's score
			player.AddScore(pickup.GetValue());
			pickup.Destroy();
		}
		//else if(MatchesCategories(pair, Category::Type::kPlayerAircraft, Category::Type::kEnemyAircraft))
		//{
		//	auto& player = static_cast<Aircraft&>(*pair.first);
		//	auto& enemy = static_cast<Aircraft&>(*pair.second);
		//	//Collision
		//	player.Damage(enemy.GetHitPoints());
		//	enemy.Destroy();
		//}

		//else if (MatchesCategories(pair, Category::Type::kPlayerAircraft, Category::Type::kPickup))
		//{
		//	auto& player = static_cast<Character&>(*pair.first);
		//	auto& pickup = static_cast<Pickup&>(*pair.second);
		//	//Apply the pickup effect
		//	pickup.Apply(player);
		//	pickup.Destroy();
		//	player.PlayLocalSound(m_command_queue, SoundEffect::kCollectPickup);
		//}

		//else if (MatchesCategories(pair, Category::Type::kPlayerAircraft, Category::Type::kEnemyProjectile) || MatchesCategories(pair, Category::Type::kEnemyAircraft, Category::Type::kAlliedProjectile))
		//{
		//	auto& aircraft = static_cast<Aircraft&>(*pair.first);
		//	auto& projectile = static_cast<Projectile&>(*pair.second);
		//	//Apply the projectile damage to the plane
		//	aircraft.Damage(projectile.GetDamage());
		//	projectile.Destroy();
		//}


	}
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Type::kEnemyAircraft | Category::Type::kProjectile;
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time)
	{
		//Does the object intersect with the battlefield
		if (!GetBattlefieldBounds().intersects(e.GetBoundingRect()))
		{
			e.Remove();
		}
	});
	m_command_queue.Push(command);
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Displays remaining time in the game as minutes and seconds
/// </summary>
void World::DisplayRemainingGameTime(float remaining_time)
{
	int minutes = (int)(remaining_time / 60);
	int seconds = (int)(remaining_time) % 60;
	
	m_game_timer_display->SetString(std::to_string(minutes) + ":" + std::to_string(seconds));
}

/// <summary>
/// Written by: Patrick Nugent
///
///	Checks if the current game has ended
/// </summary>
bool World::IsGameOver() const
{
	return m_game_over;
}

void World::UpdateSounds()
{
	sf::Vector2f listener_position;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (m_player_characters.empty())
	{
		listener_position = m_camera.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for (Character* character : m_player_characters)
		{
			listener_position += character->GetWorldPosition();
		}

		listener_position /= static_cast<float>(m_player_characters.size());
	}

	// Set listener's position
	m_sounds.SetListenerPosition(listener_position);

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Updates local players camera based on their position
/// </summary>
void World::UpdateCameraPosition()
{
	if(!m_player_characters.empty())
	{
		if(GetCharacter(m_local_player_identifier) != nullptr)
		{
			m_camera.setCenter(m_player_characters[m_local_player_identifier-1]->GetWorldPosition().x, m_camera.getCenter().y);
		}
	}
}
