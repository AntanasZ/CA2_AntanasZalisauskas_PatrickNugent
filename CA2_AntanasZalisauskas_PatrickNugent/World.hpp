/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Layers.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <array>

#include "Character.hpp"
#include "CommandQueue.hpp"
#include "PickupType.hpp"
#include "SoundPlayer.hpp"
#include "BloomEffect.hpp"
#include "NetworkNode.hpp"

namespace sf
{
	class RenderTarget;
}

/// <summary>
/// Edited By: Patrick Nugent
///
///	-Reworked to use Character class instead of Aircraft
/// -Added enemy spawn countdown fields
/// -Created separate character and pickup spawn point structs
/// -Added methods for adding and spawning pickups
/// </summary>
class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked = false);
	void Update(sf::Time dt);
	void Draw();

	CommandQueue& GetCommandQueue();
	bool IsGameOver() const;

	void SetCurrentBattleFieldPosition(float line_y);
	void SetWorldHeight(float height);

	void AddEnemy(CharacterType type, bool isFlying, float relX, float relY);
	void AddPickup(PickupType type, int value, float relX, float relY);
	
	Character* GetCharacter(int identifier) const;
	Character* AddCharacter(int identifier);
	void RemoveCharacter(int identifier);

	sf::FloatRect GetViewBounds() const;
	sf::FloatRect GetBattlefieldBounds() const;

	bool PollGameAction(GameActions::Action& out);


private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPosition();
	void AdaptPlayerVelocity(sf::Time dt);

	void SpawnEnemies();
	void SpawnFlyingEnemies();
	void SpawnPickups();
	void AddEnemies();
	void AddPickups();
	void GuideMissiles();
	void HandleCollisions();
	void DestroyEntitiesOutsideView();
	void DisplayRemainingGameTime();
	void UpdateSounds();

private:
	struct CharacterSpawnPoint
	{
		CharacterSpawnPoint(CharacterType type, float x, float y) : m_type(type), m_x(x), m_y(y)
		{
			
		}
		CharacterType m_type;
		float m_x;
		float m_y;
	};

	struct PickupSpawnPoint
	{
		PickupSpawnPoint(PickupType type, int value, float x, float y) : m_type(type), m_value(value), m_x(x), m_y(y)
		{

		}
		PickupType m_type;
		int m_value;
		float m_x;
		float m_y;
	};

private:
	sf::RenderTarget& m_target;
	sf::RenderTexture m_scene_texture;
	//sf::RenderWindow& m_window;
	sf::View m_camera;
	TextureHolder m_textures;
	FontHolder& m_fonts;
	SoundPlayer& m_sounds;
	SceneNode m_scenegraph;
	std::array<SceneNode*, static_cast<int>(Layers::kLayerCount)> m_scene_layers;
	CommandQueue m_command_queue;

	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position;
	float m_scrollspeed;
	std::vector<CharacterSpawnPoint> m_enemy_spawn_points;
	std::vector<CharacterSpawnPoint> m_flying_enemy_spawn_points;
	std::vector<PickupSpawnPoint> m_pickup_spawn_points;
	std::vector<Character*>	m_active_enemies;
	Character* m_player_character_1;
	Character* m_player_character_2;
	std::vector<Character*> m_player_characters;
	float m_gravity;
	sf::Time m_enemy_spawn_countdown;
	sf::Time m_flying_enemy_spawn_countdown;
	sf::Time m_pickup_spawn_countdown;
	sf::Time m_player_1_stun_countdown;
	sf::Time m_player_2_stun_countdown;
	sf::Time m_game_countdown;
	sf::Time m_gameover_countdown;
	TextNode* m_game_timer_display;
	bool m_game_over;

	BloomEffect m_bloom_effect;
	bool m_networked_world;
	NetworkNode* m_network_node;
	SpriteNode* m_finish_sprite;
};

