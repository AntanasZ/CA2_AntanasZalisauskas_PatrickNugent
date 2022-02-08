/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include <SFML/Graphics/Sprite.hpp>

#include "Entity.hpp"
#include "PickupType.hpp"
#include "ResourceIdentifiers.hpp"

class Character;

class Pickup : public Entity
{
public:
	Pickup(PickupType type, int value, const TextureHolder& textures);
	virtual unsigned int GetCategory() const override;
	virtual sf::FloatRect GetBoundingRect() const;
	void Apply(Character& player) const;
	float GetMaxSpeed() const;
	int GetValue() const;
	virtual void DrawCurrent(sf::RenderTarget&, sf::RenderStates states) const override;
	void UpdateMovementPattern(sf::Time dt);

private:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	PickupType m_type;
	sf::Sprite m_sprite;

	bool m_is_marked_for_removal;
	float m_travelled_distance;
	int m_directions_index;
	int m_value;
};

