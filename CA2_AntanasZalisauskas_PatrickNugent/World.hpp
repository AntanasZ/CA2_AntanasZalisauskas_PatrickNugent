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
#include "NetworkNode.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <array>
#include <SFML/Graphics/RenderWindow.hpp>

#include "BloomEffect.hpp"
#include "Character.hpp"
#include "CommandQueue.hpp"
#include "SoundPlayer.hpp"

#include "NetworkProtocol.hpp"
#include "PickupType.hpp"
#include "PlayerAction.hpp"
#include "Pickup.hpp"

namespace sf
{
	class RenderTarget;
}



class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked=false);
	void Update(sf::Time dt);
	void Draw();

	sf::FloatRect GetViewBounds() const;
	CommandQueue& GetCommandQueue();

	Character* AddCharacter(int identifier, CharacterType type, bool local_player);
	void RemoveCharacter(int identifier);
	void SetCurrentBattleFieldPosition(float line_y);
	void SetWorldHeight(float height);

	void AddEnemy(CharacterType type, bool isFlying, float rel_x, float rel_y);
	void SpawnEnemies(sf::Int8 enemyType);
	void SpawnFlyingEnemies(sf::Int8 enemyType);
	void SpawnPickups(sf::Int8 pickupType, sf::Int16 pickupPosition, sf::Int16 pickupIdentifier);
	bool HasAlivePlayer() const;
	bool HasPlayerReachedEnd() const;

	void SetWorldScrollCompensation(float compensation);
	Character* GetCharacter(int identifier) const;
	Pickup* GetPickup(int identifier) const;
	sf::FloatRect GetBattlefieldBounds() const;
	void AddPickup(PickupType type, int value, float relX, float relY);
	void RemovePickup(sf::Int16 pickupIdentifier);
	bool PollGameAction(GameActions::Action& out);
	bool IsGameOver() const;
	void StunPlayer(int identifier);

	void DisplayRemainingGameTime(float remaining_time);
	void DisplayWinner();

private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPosition();
	void AdaptPlayerVelocity(sf::Time dt);
	int GetPlayerIndex(int identifier);
	
	void AddEnemies();
	void AddPickups();
	void HandleCollisions();
	void DestroyEntitiesOutsideView();
	void UpdateSounds();

	void UpdateCameraPosition();

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
	float m_scrollspeed_compensation;
	std::vector<Character*> m_player_characters;
	std::vector<CharacterSpawnPoint> m_enemy_spawn_points;
	std::vector<CharacterSpawnPoint> m_flying_enemy_spawn_points;
	std::vector<PickupSpawnPoint> m_pickup_spawn_points;
	std::vector<Character*>	m_active_enemies;

	BloomEffect m_bloom_effect;
	bool m_networked_world;
	NetworkNode* m_network_node;
	SpriteNode* m_finish_sprite;

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
	bool m_game_ending;
	int m_local_player_identifier;
	std::vector<Pickup*> m_active_pickups;
};

