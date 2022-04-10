/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "SoundNode.hpp"

#include "SoundNode.hpp"
#include "SoundPlayer.hpp"


SoundNode::SoundNode(SoundPlayer& player)
	: m_sounds(player)
{
}

void SoundNode::PlaySound(SoundEffect sound, sf::Vector2f position)
{
	m_sounds.Play(sound, position);
}

unsigned int SoundNode::GetCategory() const
{
	return Category::kSoundEffect;
}