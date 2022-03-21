#pragma once
#include <functional>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>

#include "Character.hpp"
#include "ResourceIdentifiers.hpp"

class Aircraft;

struct Direction
{
	Direction(float angle, float distance)
		: m_angle(angle), m_distance(distance)
	{
	}
	float m_angle;
	float m_distance;
};

struct AircraftData
{
	int m_hitpoints;
	float m_speed;
	Textures m_texture;
	sf::IntRect m_texture_rect;
	sf::Time m_fire_interval;
	std::vector<Direction> m_directions;
	bool m_has_roll_animation;
};

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Data related to a character
/// </summary>

/// <summary>
/// edited by: Patrick Nugent
///
///	-Added directions field
/// -Added bool field for roll animation
/// </summary>
struct CharacterData
{
	int m_hitpoints;
	float m_speed;
	float m_jump_height;
	Textures m_texture;
	std::vector<Direction> m_directions;
	bool m_has_run_animation;
};

struct PlatformData
{
	Textures m_texture;
};

struct ProjectileData
{
	int m_damage;
	float m_speed;
	Textures m_texture;
	sf::IntRect m_texture_rect;
};

/// <summary>
/// edited by: Patrick Nugent
///
///	Added speed and directions fields
/// </summary>
struct PickupData
{
	std::function<void(Character&)> m_action;
	Textures m_texture;
	//sf::IntRect m_texture_rect;
	std::vector<Direction> m_directions;
	float m_speed;
};

struct ParticleData
{
	sf::Color						m_color;
	sf::Time						m_lifetime;
};


std::vector<AircraftData> InitializeAircraftData();
std::vector<ProjectileData> InitializeProjectileData();
std::vector<PickupData> InitializePickupData();
std::vector<ParticleData> InitializeParticleData();

std::vector<CharacterData> InitializeCharacterData();
std::vector<PlatformData> InitializePlatformData();
