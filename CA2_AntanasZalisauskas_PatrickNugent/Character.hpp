/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <SFML/Graphics/Sprite.hpp>

#include "CharacterType.hpp"
#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"
#include "TextNode.hpp"
#include "Animation.hpp"

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Contains methods and variables related to a character
///
/// Edited by: Patrick Nugent
///
///	-Added method for flipping sprites
/// -Added a getter for score
/// </summary>
class Character : public Entity
{
public:
	Character(CharacterType type, TextureHolder& textures, const FontHolder& fonts, int identifier, bool isLocalCharacter);
	Character(CharacterType type, TextureHolder& textures);
	unsigned int GetCategory() const override;

	int GetIdentifier();
	void SetIdentifier(int identifier);

	void UpdateMovementPattern(sf::Time dt);
	float GetMaxSpeed() const;
	bool GetCanJump() const;
	float GetJumpHeight() const;
	int GetScore() const;
	void ToggleCanJump(bool value);
	void FlipSprite();
	void SetSprites(bool isResetting);
	void AddScore(int points);
	int GetScore();
	void SetScore(int score);
	bool GetStunned();
	void SetStunned(bool value);
	bool GetInvulnerable();
	void SetInvulnerable(bool value);
	CharacterType GetType();
	void SetType(CharacterType type);
	sf::Time GetStunTimer();
	void AddToStunTimer(sf::Time seconds);
	void ResetStunTimer();

	sf::FloatRect GetBoundingRect() const override;
	bool IsMarkedForRemoval() const override;
	void Remove() override;
	void PlayLocalSound(CommandQueue& commands, SoundEffect effect);

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	bool isPlayer() const;
	void UpdateScore() const;
	sf::Color DetermineDisplayColor();

private:
	TextureHolder& m_texture_holder;
	CharacterType m_type;
	sf::Sprite m_sprite;
	Animation m_stunned;
	Animation m_running;
	bool m_can_jump;
	float m_jump_height;
	int m_score;
	TextNode* m_score_display;
	bool m_is_stunned;
	bool m_show_stun;
	bool m_is_invulnerable;
	sf::Time m_stun_timer;
	bool m_is_facing_right;
	bool m_is_local_character;

	bool m_is_marked_for_removal;
	float m_travelled_distance;
	int m_directions_index;

	int m_identifier;
	bool m_has_reset;
};

