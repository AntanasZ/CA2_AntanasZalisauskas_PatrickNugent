/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#include "Character.hpp"

#include <iostream>

#include <SFML/Graphics/RenderTarget.hpp>

#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "SoundNode.hpp"
#include "Utility.hpp"

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Implementation of Character.hpp methods
///
/// Edited by: Patrick Nugent
///
///	Added fields and logic for stun animations
/// </summary>

namespace
{
	const std::vector<CharacterData> Table = InitializeCharacterData();
}

Textures ToTextureID(CharacterType type)
{
	switch (type)
	{
	case CharacterType::kShaggy:
		return Textures::kShaggy;
	case CharacterType::kScooby:
		return Textures::kScooby;
	case CharacterType::kFred:
		return Textures::kFred;
	case CharacterType::kVelma:
		return Textures::kVelma;
	case CharacterType::kDaphne:
		return Textures::kDaphne;
	}
	return Textures::kShaggy;
}

/// <summary>
/// Edited by: Antanas Zalisauskas
///
///	-Added text to display score for both players
///	-Added different color to score based on the player
///
/// Edited by: Patrick Nugent
///
///	-Added field for a stun animation
///	-Added field for showing stun animation
///	-Added field for running animation
/// </summary>
Character::Character(CharacterType type, TextureHolder& textures, const FontHolder& fonts, int identifier, bool isLocalCharacter)
	: Entity(Table[static_cast<int>(type)].m_hitpoints),
	m_texture_holder(textures),
	m_type(type),
	m_sprite(),
	m_stunned(),
	m_running(),
	m_show_stun(true),
	m_can_jump(true),
	m_jump_height(Table[static_cast<int>(type)].m_jump_height),
	m_is_invulnerable(false),
	m_is_facing_right(true),
	m_stun_timer(),
	m_is_marked_for_removal(false),
	m_identifier(0),
	m_is_local_character(isLocalCharacter),
	m_has_reset(false)
{
	if (m_is_local_character)
	{
		m_sprite = sf::Sprite(textures.Get(Table[static_cast<int>(type)].m_texture));
		Utility::CentreOrigin(m_sprite);
	}
	m_identifier = identifier;

	if(type == CharacterType::kShaggy || type == CharacterType::kScooby || type == CharacterType::kFred
		|| type == CharacterType::kVelma || type == CharacterType::kDaphne)
	{
		std::unique_ptr<TextNode> scoreDisplay(new TextNode(fonts, ""));

		m_stunned.SetNumFrames(4);
		m_stunned.SetDuration(sf::seconds(1));

		m_running.SetDuration(sf::seconds(1));
	    m_running.SetRepeating(true);

		SetSprites(false);

		scoreDisplay->SetColor(DetermineDisplayColor());
		scoreDisplay->setPosition(0, -55);
		m_score_display = scoreDisplay.get();
		AttachChild(std::move(scoreDisplay));
		UpdateScore();
	}
}

/// <summary>
/// Created by: Patrick Nugent
///
/// Overloaded constructor for characters that don't belong to players
/// </summary>
Character::Character(CharacterType type, TextureHolder& textures)
	: Entity(Table[static_cast<int>(type)].m_hitpoints),
	m_texture_holder(textures),
	m_type(type),
	m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture)),
	m_is_marked_for_removal(false),
	m_identifier(0)
{
	Utility::CentreOrigin(m_sprite);
}

/// <summary>
/// Edited by: Antanas Zalisauskas
///
/// Edited to work with player and enemy character categories
/// </summary>
/// <returns></returns>
unsigned Character::GetCategory() const
{
	if (isPlayer())
		return static_cast<int>(Category::kPlayerCharacter);
	else
		return static_cast<int>(Category::kEnemyCharacter);
}

//********* Implement Later for Enemies ****************//
void Character::UpdateMovementPattern(sf::Time dt)
{
	//Enemy AI
	const std::vector<Direction>& directions = Table[static_cast<int>(m_type)].m_directions;
	if (!directions.empty())
	{
		//Move along the current direction, change direction
		if (m_travelled_distance > directions[m_directions_index].m_distance)
		{
			m_directions_index = (m_directions_index + 1) % directions.size();
			m_travelled_distance = 0.f;
		}

		//Compute velocity from direction
		double radians = Utility::ToRadians(directions[m_directions_index].m_angle + 90.f);
		float vx = GetMaxSpeed() * std::cos(radians);
		float vy = GetMaxSpeed() * std::sin(radians);

		SetVelocity(vx, vy);
		m_travelled_distance += GetMaxSpeed() * dt.asSeconds();
	}
}
//********* Implement Later for Enemies ****************//

float Character::GetMaxSpeed() const
{
	return Table[static_cast<int>(m_type)].m_speed;
}

sf::FloatRect Character::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

bool Character::IsMarkedForRemoval() const
{
	return m_is_marked_for_removal;
}

void Character::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (m_is_stunned && m_show_stun)
	{
		target.draw(m_stunned, states);
	}
	else if (Table[static_cast<int>(m_type)].m_has_run_animation)
	{
		target.draw(m_running, states);
	}
	else
	{
		target.draw(m_sprite, states);
	}
}

/// <summary>
/// Edited by: Patrick Nugent
///
/// Added checks for flipping player sprites based on velocity and 
/// the direction that they're currently facing
/// </summary>
void Character::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	if (IsDestroyed())
	{
		m_is_marked_for_removal = true;
		return;
	}
	else if (m_is_stunned)
	{
		//Update stun animation as long as it isn't on the last frame
		if (!m_stunned.IsFinished())
		{
			m_stunned.Update(dt);
		}
	}
	else if (Table[static_cast<int>(m_type)].m_has_run_animation)
	{
		m_running.Update(dt);
	}

	UpdateMovementPattern(dt);
	Entity::UpdateCurrent(dt, commands);

	if (isPlayer())
	{
		const sf::Vector2f velocity = GetVelocity();
		if (velocity.x > 0 && !m_is_facing_right)
		{
			m_is_facing_right = true;
			FlipSprite();
			m_running.scale(-1, 1);
			m_stunned.scale(-1, 1);
		}
		else if (velocity.x < 0 && m_is_facing_right)
		{
			m_is_facing_right = false;
			FlipSprite();
			m_running.scale(-1, 1);
			m_stunned.scale(-1, 1);
		}
	}
}

bool Character::isPlayer() const
{
	if (m_type == CharacterType::kShaggy || m_type == CharacterType::kScooby || 
		m_type == CharacterType::kFred || m_type == CharacterType::kVelma || m_type == CharacterType::kDaphne)
		return true;
	else
		return false;
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Added getters and setters for jump variables
/// </summary>
/// <returns></returns>
bool Character::GetCanJump() const
{
	return m_can_jump;
}

float Character::GetJumpHeight() const
{
	return m_jump_height;
}

/// <summary>
/// Written by: Patrick Nugent
///
///	Added getter for score variable
/// </summary>
int Character::GetScore() const
{
	return m_score;
}

int	Character::GetIdentifier()
{
	return m_identifier;
}

void Character::SetIdentifier(int identifier)
{
	m_identifier = identifier;
}

void Character::ToggleCanJump(bool value)
{
	m_can_jump = value;
}

/// <summary>
/// Written by: Patrick Nugent
///
///	Flips the sprite of a character
/// </summary>
/// <returns></returns>
void Character::FlipSprite()
{
	m_sprite.scale(-1, 1);
}

/// <summary>
/// Written by: Patrick Nugent
///
///	Sets sprites of a character and changes it to match the
/// current character type
/// </summary>
void Character::SetSprites(bool isResetting)
{
	if (m_type == CharacterType::kShaggy)
	{
		m_stunned.SetTexture(m_texture_holder.Get(Textures::kShaggyStunned));
		m_stunned.SetFrameSize(sf::Vector2i(30, 69));

		m_running.SetNumFrames(10);
		m_running.SetTexture(m_texture_holder.Get(Textures::kShaggyRunning));
		m_running.SetFrameSize(sf::Vector2i(59, 68));
	}
	else if (m_type == CharacterType::kScooby)
	{
		m_stunned.SetTexture(m_texture_holder.Get(Textures::kScoobyStunned));
		m_stunned.SetFrameSize(sf::Vector2i(49, 46));

		m_running.SetNumFrames(7);
		m_running.SetTexture(m_texture_holder.Get(Textures::kScoobyRunning));
		m_running.SetFrameSize(sf::Vector2i(67, 46));
	}
	else if (m_type == CharacterType::kFred)
	{
		m_stunned.SetTexture(m_texture_holder.Get(Textures::kFredStunned));
		m_stunned.SetFrameSize(sf::Vector2i(27, 67));

		m_running.SetNumFrames(12);
		m_running.SetTexture(m_texture_holder.Get(Textures::kFredRunning));
		m_running.SetFrameSize(sf::Vector2i(49, 67));
	}
	else if (m_type == CharacterType::kVelma)
	{
		m_stunned.SetTexture(m_texture_holder.Get(Textures::kVelmaStunned));
		m_stunned.SetFrameSize(sf::Vector2i(38, 59));

		m_stunned.SetNumFrames(3);

		m_running.SetNumFrames(12);
		m_running.SetTexture(m_texture_holder.Get(Textures::kVelmaRunning));
		m_running.SetFrameSize(sf::Vector2i(36, 59));
	}
	else if (m_type == CharacterType::kDaphne)
	{
		m_stunned.SetTexture(m_texture_holder.Get(Textures::kDaphneStunned));
		m_stunned.SetFrameSize(sf::Vector2i(25, 59));

		m_stunned.SetNumFrames(3);

		m_running.SetNumFrames(12);
		m_running.SetTexture(m_texture_holder.Get(Textures::kDaphneRunning));
		m_running.SetFrameSize(sf::Vector2i(45, 59));
	}
		
	if (!m_is_local_character && isResetting)
	{
		m_sprite = sf::Sprite(m_texture_holder.Get(Table[static_cast<int>(m_type)].m_texture));
		Utility::CentreOrigin(m_sprite);
		Utility::CentreOrigin(m_running);
		Utility::CentreOrigin(m_stunned);
	}
	else if(m_is_local_character)
	{
		Utility::CentreOrigin(m_running);
		Utility::CentreOrigin(m_stunned);
	}

}

/// <summary>
/// Written By: Antanas Zalisauskas
///
///	Updates player score
/// </summary>
void Character::UpdateScore() const
{
	m_score_display->SetString("Player " + std::to_string(m_identifier) + "\n\t " + std::to_string(m_score));
}

/// <summary>
/// Written By: Antanas Zalisauskas
///
///	Adds a specified amount to player's score
/// </summary>
/// <param name="points"> The number of points to add to player's score </param>
void Character::AddScore(int points)
{
	m_score += points;
	UpdateScore();
}


/// <summary>
/// Written By: Patrick Nugent
///
///	Added getter and setter for character score
/// </summary>
int Character::GetScore()
{
	return m_score;
}

void Character::SetScore(int score)
{
	m_score = score;
	UpdateScore();
}

/// <summary>
/// Written By: Antanas Zalisauskas
///
/// -Added getter and setter for player stun mechanic
/// -Added invulnerability methods
///
/// Edited By: Patrick Nugent
///
/// -Added call to restart stun animation in setter
/// </summary>
/// <returns></returns>
bool Character::GetStunned()
{
	return m_is_stunned;
}

void Character::SetStunned(bool value)
{
	m_is_stunned = value;
	m_stunned.Restart();
}

bool Character::GetInvulnerable()
{
	return m_is_invulnerable;
}

void Character::SetInvulnerable(bool value)
{
	m_is_invulnerable = value;
}

CharacterType Character::GetType()
{
	return m_type;
}

void Character::SetType(CharacterType type)
{
	/*if (m_type == CharacterType::kShaggy && type == CharacterType::kShaggy && !m_has_reset)
	{
		m_has_reset = true;
		m_type = type;
		SetSprites(true);
	}*/
	if (m_type != type)
	{
		m_type = type;
		SetSprites(true);
	}
	
	/*if (!m_has_reset)
	{
		m_has_reset = true;
		m_type = type;
		SetSprites(true);
	}*/
}

void Character::Remove()
{
	Entity::Remove();
}

void Character::PlayLocalSound(CommandQueue& commands, SoundEffect effect)
{
	sf::Vector2f world_position = GetWorldPosition();

	Command command;
	command.category = Category::kSoundEffect;
	command.action = DerivedAction<SoundNode>(
		[effect, world_position](SoundNode& node, sf::Time)
		{
			node.PlaySound(effect, world_position);
		});

	commands.Push(command);
}

sf::Time Character::GetStunTimer()
{
	return m_stun_timer;
}

void Character::AddToStunTimer(sf::Time seconds)
{
	m_stun_timer += seconds;
}

void Character::ResetStunTimer()
{
	m_stun_timer = sf::Time::Zero;
}

/// <summary>
/// Edited by: Patrick Nugent
///
/// Returns a color to use for text based on the player's id
/// </summary>
/// <returns>The color associated with the player's identifier</returns>
sf::Color Character::DetermineDisplayColor()
{
	sf::Color color;

	switch (m_identifier)
	{
		case 1:
		{
			return sf::Color::Green;
		}
		break;
		case 2:
		{
			return sf::Color::Red;
		}
		break;
		case 3:
		{
			return sf::Color::Blue;
		}
		break;
		case 4:
		{
			return sf::Color::Yellow;
		}
		break;
		case 5:
		{
			//orange
			return sf::Color::Color(255, 165, 0, 255);
		}
		break;
		case 6:
		{
			//purple
			return sf::Color::Color(128, 0, 128, 255);
		}
		break;
		case 7:
		{
			//pink
			return sf::Color::Color(255, 192, 203, 255);
		}
		break;
		case 8:
		{
			//grey
			return sf::Color::Color(192, 192, 192, 255);
		}
		break;
		case 9:
		{
			return sf::Color::Cyan;
		}
		break;
		case 10:
		{
			return sf::Color::Color::Magenta;
		}
		break;
		case 11:
		{
			//brown
			return sf::Color::Color(153, 76, 0, 255);
		}
		break;
		case 12:
		{
			//fuchsia
			return sf::Color::Color(255, 0, 255, 255);
		}
		break;
		case 13:
		{
			return sf::Color::White;
		}
		break;
		case 14:
		{
			//dark green
			return sf::Color::Color(8, 59, 21, 255);
		}
		break;
		case 15:
		{
			//beige
			return sf::Color::Color(245, 245, 220, 255);
		}
		break;
		default: return sf::Color::Color(245, 245, 220, 255);
	}
}