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
	, m_world_bounds(0.f, 0.f, 3072.f/*m_camera.getSize().x*/, m_camera.getSize().y)//5000.f)
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
	if (!m_game_ending)
	{
		for (Character* a : m_player_characters)
		{
			a->SetVelocity(0.f, a->GetVelocity().y);
		}

		DestroyEntitiesOutsideView();

		//Forward commands to the scenegraph until the command queue is empty
		while (!m_command_queue.IsEmpty())
		{
			m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
		}
		AdaptPlayerVelocity(dt);

		HandleCollisions();

		auto first_to_remove = std::remove_if(m_player_characters.begin(), m_player_characters.end(), std::mem_fn(&Character::IsMarkedForRemoval));
		m_player_characters.erase(first_to_remove, m_player_characters.end());
		m_scenegraph.RemoveWrecks();

		for (Character* a : m_player_characters)
		{
			if (a->GetInvulnerable())
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
	}
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
	if (local_player)
	{
		m_local_player_identifier = identifier;
	}

	std::unique_ptr<Character> player(new Character(type, m_textures, m_fonts, identifier, local_player));
	player->setPosition(m_camera.getCenter());
	//player->SetIdentifier(identifier);
	m_player_characters.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(player));

	return m_player_characters.back();
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
	m_textures.Load(Textures::kParticle, "Media/Textures/Particle.png");
	m_textures.Load(Textures::kBackground, "Media/Textures/Background.png");
	m_textures.Load(Textures::kPlatformForest, "Media/Textures/PlatformForest.png");
	m_textures.Load(Textures::kPlatformCastle, "Media/Textures/PlatformCastle.png");

	m_textures.Load(Textures::kPlaceholderCharacter, "Media/Textures/PlaceholderCharacter.png");
	m_textures.Load(Textures::kPlaceholderCharacterRunning, "Media/Textures/PlaceholderCharacterRunning.png");
	m_textures.Load(Textures::kPlaceholderCharacterStunned, "Media/Textures/PlaceholderCharacterStunned.png");
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
	sf::Texture& texture = m_textures.Get(Textures::kBackground);
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

	//Prepare platforms
	std::unique_ptr<Platform> ground_platform(new Platform(PlatformType::kGroundPlatform, m_textures));
	ground_platform->setPosition(m_world_bounds.width/2, 755.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(ground_platform));

	std::unique_ptr<Platform> platformForest(new Platform(PlatformType::kAirPlatformForest, m_textures));
	platformForest->setPosition(250.f, 450.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformForest));

	std::unique_ptr<Platform> platformForest2(new Platform(PlatformType::kAirPlatformForest, m_textures));
	platformForest2->setPosition(750.f, 450.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformForest2));

	std::unique_ptr<Platform> platformForest3(new Platform(PlatformType::kAirPlatformForest, m_textures));
	platformForest3->setPosition(500.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformForest3));

	std::unique_ptr<Platform> platformMansion(new Platform(PlatformType::kAirPlatform, m_textures));
	platformMansion->setPosition(m_world_bounds.width / 2, 450.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformMansion));

	std::unique_ptr<Platform> platformMansion2(new Platform(PlatformType::kAirPlatform, m_textures));
	platformMansion2->setPosition((m_world_bounds.width / 2) + 250.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformMansion2));

	std::unique_ptr<Platform> platformMansion3(new Platform(PlatformType::kAirPlatform, m_textures));
	platformMansion3->setPosition((m_world_bounds.width / 2) - 250.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformMansion3));

	std::unique_ptr<Platform> platformMansion4(new Platform(PlatformType::kAirPlatform, m_textures));
	platformMansion4->setPosition((m_world_bounds.width / 2) + 250.f, 300.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformMansion4));

	std::unique_ptr<Platform> platformMansion5(new Platform(PlatformType::kAirPlatform, m_textures));
	platformMansion5->setPosition((m_world_bounds.width / 2) - 250.f, 300.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformMansion5));

	std::unique_ptr<Platform> platformCastle(new Platform(PlatformType::kAirPlatformCastle, m_textures));
	platformCastle->setPosition(2550.f, 450.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformCastle));

	std::unique_ptr<Platform> platformCastle2(new Platform(PlatformType::kAirPlatformCastle, m_textures));
	platformCastle2->setPosition(2300.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformCastle2));

	std::unique_ptr<Platform> platformCastle3(new Platform(PlatformType::kAirPlatformCastle, m_textures));
	platformCastle3->setPosition(2800.f, 600.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(platformCastle3));

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
	const float border_distance = 35.f;

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
	sf::Vector2f position((m_world_bounds.width / 2) - m_world_bounds.width / 2.f, (m_world_bounds.height / 2) - m_world_bounds.height / 2.f);
	sf::Vector2f size(m_world_bounds.width, m_camera.getSize().y);
	
	return sf::FloatRect(position, size);
}

sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return world bounds + a small area at the top where enemies spawn offscreen
	sf::FloatRect bounds = m_world_bounds;//GetViewBounds();
	bounds.left -= 100.f;
	bounds.width += 100.f;

	return bounds;
}

void World::SpawnEnemies(sf::Int8 enemyType)
{
	//Spawn a random enemy from the vector of enemy spawn points
	CharacterSpawnPoint spawn = m_enemy_spawn_points[enemyType];
	std::unique_ptr<Character> enemy(new Character(spawn.m_type, m_textures));
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
	std::unique_ptr<Character> enemy(new Character(spawn.m_type, m_textures));
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
	AddEnemy(CharacterType::kCreeperRight, false, 2500.f, -329.5f);
	AddEnemy(CharacterType::kMichaelLeft, false, -500.f, -325.f);
	AddEnemy(CharacterType::kMichaelRight, false, 2500.f, -325.f);
	AddEnemy(CharacterType::kFreddyLeft, false, -500.f, -323.f);
	AddEnemy(CharacterType::kFreddyRight, false, 2500.f, -323.f);
	AddEnemy(CharacterType::kJasonLeft, false, -500.f, -321.f);
	AddEnemy(CharacterType::kJasonRight, false, 2500.f, -321.f);
	AddEnemy(CharacterType::kStripeLeft, false, -500.f, -335.f);
	AddEnemy(CharacterType::kStripeRight, false, 2500.f, -335.f);
	AddEnemy(CharacterType::kGarfieldLeft, false, -500.f, -331.f);
	AddEnemy(CharacterType::kGarfieldRight, false, 2500.f, -331.f);

	AddEnemy(CharacterType::kGhidorahLeft, true, -500.f, -2.f);
	AddEnemy(CharacterType::kGhidorahRight, true, 2500.f, -2.f);
	AddEnemy(CharacterType::kPterodactylLeft, true, -500.f, -2.f);
	AddEnemy(CharacterType::kPterodactylRight, true, 2500.f, -2.f);
	AddEnemy(CharacterType::kTurtleLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kTurtleRight, true, 2500.f, -150.f);
	AddEnemy(CharacterType::kGhostLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kGhostRight, true, 2500.f, -150.f);
	AddEnemy(CharacterType::kSkullLeft, true, -500.f, -150.f);
	AddEnemy(CharacterType::kSkullRight, true, 2500.f, -150.f);
	AddEnemy(CharacterType::kDutchmanLeft, true, -500.f, 200.f);
	AddEnemy(CharacterType::kDutchmanRight, true, 2500.f, 200.f);
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
	float yPosition = 380.f;

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
		if (MatchesCategories(pair, Category::Type::kPlatform, Category::Type::kPlayerCharacter))
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
		else if (MatchesCategories(pair, Category::Type::kPlayerCharacter, Category::Type::kEnemyCharacter))
		{
			auto& player = static_cast<Character&>(*pair.first);
			if (!player.GetInvulnerable())
			{
				m_sounds.Play(SoundEffect::kStun);
				player.SetStunned(true);
				player.SetInvulnerable(true);
			}
		}
		else if (MatchesCategories(pair, Category::Type::kPlayerCharacter, Category::Type::kPickup))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			m_sounds.Play(SoundEffect::kCollectPickup);

			//Add the pickup's value to the player's score
			player.AddScore(pickup.GetValue());
			pickup.Destroy();

			//m_network_node->NotifyGameAction(GameActions::CollectPickup, pickup.GetValue());
		}
		else if(MatchesCategories(pair, Category::Type::kPlayerCharacter, Category::Type::kAlliedProjectile))
		{
			auto& player = static_cast<Character&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			if (!player.GetInvulnerable())
			{
				m_sounds.Play(SoundEffect::kStun);
				player.SetStunned(true);
				player.SetInvulnerable(true);
				projectile.Destroy();
			}
		}
		else if(MatchesCategories(pair, Category::Type::kAlliedProjectile, Category::Type::kPlatform))
		{
			auto& projectile = static_cast<Projectile&>(*pair.first);
			projectile.Destroy();
		}
	}
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Type::kEnemyCharacter | Category::Type::kProjectile | Category::Type::kPickup;
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
			m_camera.setCenter(m_player_characters[GetPlayerIndex(m_local_player_identifier)]->GetWorldPosition().x, m_camera.getCenter().y);
			m_game_timer_display->setPosition(m_player_characters[GetPlayerIndex(m_local_player_identifier)]->GetWorldPosition().x, m_game_timer_display->GetWorldPosition().y);
		}
	}
}

/// <summary>
/// Written by: Patrick Nugent
///
///	Draws text on the screen displaying the winner
/// </summary>
void World::DisplayWinner()
{
	m_game_ending = true;

	int highestScore = 0;
	int winningPlayer = 0;

	for (Character* character : m_player_characters)
	{
		if (character->GetScore() > highestScore)
		{
			highestScore = character->GetScore();
			winningPlayer = character->GetIdentifier();
		}
	}

	if (highestScore != 0)
	{
		m_game_timer_display->SetString("Player " + std::to_string(winningPlayer) + " wins with " + std::to_string(highestScore) + " points!");
	}
	else
	{
		m_game_timer_display->SetString("Zoinks! Looks like nobody wins!");
	}
}

int World::GetPlayerIndex(int identifier)
{
	for (int i = 0; i < m_player_characters.size(); i++)
	{
		if (m_player_characters[i]->GetIdentifier() == identifier)
		{
			return i;
		}
	}
	return -1;
}
