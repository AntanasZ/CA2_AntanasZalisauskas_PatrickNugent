/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#pragma once
#include "SceneNode.hpp"
#include "NetworkProtocol.hpp"

#include <queue>

class NetworkNode : public SceneNode
{
public:
	NetworkNode();
	void NotifyGameAction(GameActions::Type type, sf::Vector2f position);
	void NotifyGameAction(GameActions::Type type, sf::Int16 pickupIdentifier, sf::Int8 playerIdentifier);
	void NotifyGameAction(GameActions::Type type, sf::Int8 playerIdentifier);
	bool PollGameAction(GameActions::Action& out);
	virtual unsigned int GetCategory() const override;

private:
	std::queue<GameActions::Action> m_pending_actions;
};

