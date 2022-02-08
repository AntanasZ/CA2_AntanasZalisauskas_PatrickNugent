/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"


class SoundPlayer;

class SoundNode : public SceneNode
{
public:
	explicit SoundNode(SoundPlayer& player);
	void PlaySound(SoundEffect sound, sf::Vector2f position);

	virtual unsigned int GetCategory() const override;


private:
	SoundPlayer& m_sounds;
};

