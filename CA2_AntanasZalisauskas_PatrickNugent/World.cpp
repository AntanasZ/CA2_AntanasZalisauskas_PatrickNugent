/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#include "Pickup.hpp"
#include "Platform.hpp"
#include "Utility.hpp"
#include "SoundPlayer.hpp"
#include "PostEffect.hpp"

//Some logic related to jumping and gravity made with help from this tutorial
//https://www.youtube.com/watch?v=6WopQvdNRSA&ab_channel=HilzeVonck

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph()
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, m_camera.getSize().y)
	, m_spawn_position(m_camera.getSize().x/2.f, m_world_bounds.height - m_camera.getSize().y /2.f)
	, m_scrollspeed(-50.f)
	, m_player_character_1(nullptr)
	, m_player_character_2(nullptr)
	, m_gravity(981.f)
	, m_enemy_spawn_countdown()
	, m_pickup_spawn_countdown()
	, m_player_1_stun_countdown()
	, m_player_2_stun_countdown()
	, m_game_countdown(sf::seconds(120))
	, m_game_over(false)
	, m_player_characters()
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	//std::cout << m_camera.getSize().x << m_camera.getSize().y << std::endl;
	m_camera.setCenter(m_spawn_position);
}

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Added enemy and pickup spawn timers
/// -Added win message
///
/// Edited By: Antanas Zalisauskas
///
///	-Added stun timers
/// -Added game timer
/// </summary>
void World::Update(sf::Time dt)
{
	//Check if remaining game time is greater than 0
	if (m_game_countdown > sf::Time::Zero)
	{
		//Decrease and Display remaining game time
		m_game_countdown -= dt;
		DisplayRemainingGameTime();

		//m_player_aircraft->SetVelocity(0.f, 0.f);
		m_player_character_1->SetVelocity(0.f, m_player_character_1->GetVelocity().y);
		m_player_character_2->SetVelocity(0.f, m_player_character_2->GetVelocity().y);

		DestroyEntitiesOutsideView();

		//Forward commands to the scenegraph until the command queue is empty
		while (!m_command_queue.IsEmpty())
		{
			m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
		}

		AdaptPlayerVelocity(dt);

		HandleCollisions();
		//Remove all destroyed entities
		m_scenegraph.RemoveWrecks();

		//Spawn an enemy every 5 seconds and reset the spawn timer
		m_enemy_spawn_countdown += dt;
		if (m_enemy_spawn_countdown >= sf::seconds(5.0f))
		{
			SpawnEnemies();
			m_enemy_spawn_countdown = sf::seconds(0.f);
		}

		//Spawn a flying enemy every 3 seconds and reset the spawn timer
		m_flying_enemy_spawn_countdown += dt;
		if (m_flying_enemy_spawn_countdown >= sf::seconds(3.0f))
		{
			SpawnFlyingEnemies();
			m_flying_enemy_spawn_countdown = sf::seconds(0.f);
		}

		//Spawn a pickup every 1.5 seconds and reset the spawn timer
		m_pickup_spawn_countdown += dt;
		if (m_pickup_spawn_countdown >= sf::seconds(1.5f))
		{
			SpawnPickups();
			m_pickup_spawn_countdown = sf::seconds(0.f);
		}

		if (m_player_character_1->GetInvulnerable())
		{
			//un-stun the player after 3 seconds
			m_player_1_stun_countdown += dt;
			if (m_player_1_stun_countdown >= sf::seconds(3.0f))
			{
				m_player_character_1->SetStunned(false);
			}

			//enable collisions with enemies again after 5 seconds
			if (m_player_1_stun_countdown >= sf::seconds(5.0f))
			{
				m_player_character_1->SetInvulnerable(false);
				m_player_1_stun_countdown = sf::seconds(0.f);
			}
		}

		if (m_player_character_2->GetInvulnerable())
		{
			//un-stun the player after 3 seconds
			m_player_2_stun_countdown += dt;
			if (m_player_2_stun_countdown >= sf::seconds(3.0f))
			{
				m_player_character_2->SetStunned(false);
			}

			//enable collisions with enemies again after 5 seconds
			if (m_player_2_stun_countdown >= sf::seconds(5.0f))
			{
				m_player_character_2->SetInvulnerable(false);
				m_player_2_stun_countdown = sf::seconds(0.f);
			}
		}

		//Apply movement
		m_scenegraph.Update(dt, m_command_queue);
		AdaptPlayerPosition();

		UpdateSounds();
	}
	else
	{
		//End the game and wait 5 seconds while displaying the winning score
		m_game_countdown = sf::Time::Zero;
		m_gameover_countdown += dt;

		if (m_gameover_countdown >= sf::seconds(5.0f))
		{
			m_game_over = true;
		}
		else
		{
			if (m_player_character_1->GetScore() > m_player_character_2->GetScore())
			{
				m_game_timer_display->SetString("Player 1 wins with " + std::to_string(m_player_character_1->GetScore()) + " points!");
			}
			else if (m_player_character_2->GetScore() > m_player_character_1->GetScore())
			{
				m_game_timer_display->SetString("Player 2 wins with " + std::to_string(m_player_character_2->GetScore()) + " points!");
			}
			else
			{
				m_game_timer_display->SetString("It's a draw, both players have " + std::to_string(m_player_character_1->GetScore()) + " points");
			}
		}
	}
}

void World::Draw()
{
	if (PostEffect::IsSupported())
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
	//m_window.setView(m_camera);
	//m_window.draw(m_scenegraph);
}

/// <summary>
/// Edited by: Patrick Nugent
///
///	-Added enemy textures
/// -Added pickup textures
///
///	Edited by: Antanas Zalisauskas
///	-Added platform textures
///	-Added scooby and shaggy textures
///	-Added mansion texture
/// </summary>
void World::LoadTextures()
{
	m_textures.Load(Textures::kShaggy, "Media/Textures/ShaggyIdle.png");
	m_textures.Load(Textures::kShaggyStunned, "Media/Textures/ShaggyStunned.png");
	m_textures.Load(Textures::kShaggyRunning, "Media/Textures/ShaggyRunning.png");
	m_textures.Load(Textures::kScooby, "Media/Textures/ScoobyIdle.png");
	m_textures.Load(Textures::kScoobyStunned, "Media/Textures/ScoobyStunned.png");
	m_textures.Load(Textures::kScoobyRunning, "Media/Textures/ScoobyRunning.png");
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

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	-Changed method to work with m_player_character variables and added player 1 and 2 to game
///	-Changed background texture used
///	-Added platforms to the level
/// </summary>
void World::BuildScene()
{
	//Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(Layers::kLayerCount); ++i)
	{
		Category::Type category = (i == static_cast<int>(Layers::kAir)) ? Category::Type::kScene : Category::Type::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(Textures::kMansion);
	sf::IntRect textureRect(m_world_bounds);
	//Tile the texture to cover our world
	texture.setRepeated(true);

	//Add the background sprite to our scene
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top);
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(background_sprite));

	//Prepare platforms
	std::unique_ptr<Platform> ground_platform(new Platform(PlatformType::kGroundPlatform, m_textures));
	ground_platform->setPosition(m_world_bounds.width/2, 755.f);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(ground_platform));

	std::unique_ptr<Platform> platform1(new Platform(PlatformType::kAirPlatform, m_textures));
	platform1->setPosition(525.f, 450.f);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(platform1));

	std::unique_ptr<Platform> platform2(new Platform(PlatformType::kAirPlatform, m_textures));
	platform2->setPosition(800.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(platform2));

	std::unique_ptr<Platform> platform3(new Platform(PlatformType::kAirPlatform, m_textures));
	platform3->setPosition(250.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(platform3));

	//Add player character
	std::unique_ptr<Character> player1(new Character(CharacterType::kShaggy, m_textures, m_fonts));
	m_player_character_1 = player1.get();
	m_player_character_1->setPosition(m_spawn_position);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(player1));

	std::unique_ptr<Character> player2(new Character(CharacterType::kScooby, m_textures, m_fonts));
	m_player_character_2 = player2.get();
	m_player_character_2->setPosition(m_spawn_position);
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(player2));

	//Add game timer
	std::unique_ptr<TextNode> gameTimerDisplay(new TextNode(m_fonts, ""));
	gameTimerDisplay->setPosition(m_world_bounds.width / 2, 20);
	gameTimerDisplay->SetColor(sf::Color::Yellow);
	m_game_timer_display = gameTimerDisplay.get();
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(gameTimerDisplay));

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
	//Keep the player on the screen
	//sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	sf::FloatRect view_bounds = GetViewBounds();

	const float border_distance = 48.f;
	//sf::Vector2f position = m_player_character_1->GetWorldPosition();
	sf::Vector2f position = m_player_character_1->getPosition();
	position.x = std::max(position.x, view_bounds.left + border_distance);
	position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
	position.y = std::max(position.y, view_bounds.top + border_distance);
	position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance - 10.f);
	m_player_character_1->setPosition(position);

	position = m_player_character_2->getPosition();
	position.x = std::max(position.x, view_bounds.left + border_distance);
	position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
	position.y = std::max(position.y, view_bounds.top + border_distance);
	position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance);
	m_player_character_2->setPosition(position);

}

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	Added gravity to players
/// </summary>
void World::AdaptPlayerVelocity(sf::Time dt)
{
	//sf::Vector2f velocity = m_player_character_1->GetVelocity();
	////if moving diagonally then reduce velocity
	//if (velocity.x != 0.f && velocity.y != 0.f)
	//{
	//	m_player_character_1->SetVelocity(velocity / std::sqrt(2.f));
	//}

	//velocity = m_player_character_2->GetVelocity();
	////if moving diagonally then reduce velocity
	//if (velocity.x != 0.f && velocity.y != 0.f)
	//{
	//	m_player_character_2->SetVelocity(velocity / std::sqrt(2.f));
	//}

	//Add gravity to players
	m_player_character_1->Accelerate(0.f, m_gravity * dt.asSeconds());
	m_player_character_2->Accelerate(0.f, m_gravity * dt.asSeconds());

	//Add scrolling velocity
	//m_player_character->Accelerate(0.f, m_scrollspeed);
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

/// <summary>
/// Edited By: Antanas
///	changed .top & .height to .left & .width
/// </summary>
/// <returns></returns>
sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn offscreen
	sf::FloatRect bounds = GetViewBounds();
	bounds.left -= 100.f;
	bounds.width += 100.f;

	return bounds;
}

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

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Reworked to use Character class instead of aircraft
/// -Added enemy randomiser code
/// </summary>
void World::SpawnEnemies()
{
	//Spawn a random enemy from the vector of enemy spawn points
	int randomEnemy = rand() % 12;
	CharacterSpawnPoint spawn = m_enemy_spawn_points[randomEnemy];
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
/// Edited By: Patrick Nugent
///
///	-Same as SpawnEnemies but handles flying enemies instead
/// </summary>
void World::SpawnFlyingEnemies()
{
	//Spawn a random flying enemy from the vector of flying enemy spawn points
	int randomEnemy = rand() % 12;
	CharacterSpawnPoint spawn = m_flying_enemy_spawn_points[randomEnemy];
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
/// </summary>
void World::SpawnPickups()
{
	//Spawn a random pickup from the vector of pickup spawn points
	int randomPickup = rand() % 9;
	PickupSpawnPoint spawn = m_pickup_spawn_points[randomPickup];
	std::unique_ptr<Pickup> pickup(new Pickup(spawn.m_type, spawn.m_value, m_textures));

	//Generate a random x value for the pickup's position (within the bounds)
	int randomPosition = (rand() % 934) + 90;
	pickup->setPosition((float)randomPosition, spawn.m_y);

	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(pickup));
}

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Changed to add to a specific enemy list depending on type
/// </summary>
void World::AddEnemy(CharacterType type, bool isFlying, float relX, float relY)
{
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

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Added enemy types
/// </summary>
void World::AddEnemies()
{
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
}

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

Character* World::GetCharacter(int identifier) const
{
	for (Character* a : m_player_characters)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

Character* World::AddCharacter(int identifier)
{
	std::unique_ptr<Character> player(new Character(CharacterType::kShaggy, m_textures, m_fonts));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);

	m_player_characters.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(player));
	return m_player_characters.back();
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

bool MatchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();

	//std::cout << category1 << category2 << std::endl;

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

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	-Added Collision between players and platforms
///	-Added Collision between players and enemies
///
/// Edited by: Patrick Nugent
///
///	-Added Collision between players and pickups
/// -Added sounds for enemy and pickup collisions
/// </summary>
void World::HandleCollisions()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for(SceneNode::Pair pair : collision_pairs)
	{
	
		if(MatchesCategories(pair, Category::Type::kPlatform, Category::Type::kPlayerCharacter1))
		{
			auto& platform = static_cast<Platform&>(*pair.first);
			auto& player = static_cast<Character&>(*pair.second);

			if(player.GetWorldPosition().y < platform.GetWorldPosition().y)
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

		else if(MatchesCategories(pair, Category::Type::kPlayerCharacter1, Category::Type::kEnemyCharacter) || MatchesCategories(pair, Category::Type::kPlayerCharacter2, Category::Type::kEnemyCharacter))
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

	}
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Type::kEnemyCharacter | Category::Type::kPickup;
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time)
	{
		//Does the object intersect with the battlefield
		if (!GetBattlefieldBounds().intersects(e.GetBoundingRect()))
		{
			//std::cout << "Destroying Entity" << std::endl;
			e.Destroy();
		}
	});
	m_command_queue.Push(command);
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Displays remaining time in the game as minutes and seconds
/// </summary>
void World::DisplayRemainingGameTime()
{
	int minutes = (int)(m_game_countdown.asSeconds() / 60);
	int seconds = (int)(m_game_countdown.asSeconds()) % 60;
	
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
	// Set listener's position to player position
	m_sounds.SetListenerPosition(m_player_character_1->GetWorldPosition());

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}
