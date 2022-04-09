#include "NetworkNode.hpp"

/// <summary>
/// Edited by: Patrick Nugent
///
/// Added overloaded constructor for NotifyGameAction which takes in value
/// </summary>

NetworkNode::NetworkNode()
: SceneNode()
, m_pending_actions()
{
}

void NetworkNode::NotifyGameAction(GameActions::Type type, sf::Vector2f position)
{
	m_pending_actions.push(GameActions::Action(type, position));
}


void NetworkNode::NotifyGameAction(GameActions::Type type, sf::Int16 pickupIdentifier, sf::Int8 playerIdentifier)
{
	m_pending_actions.push(GameActions::Action(type, pickupIdentifier, playerIdentifier));
}

void NetworkNode::NotifyGameAction(GameActions::Type type, sf::Int8 playerIdentifier)
{
	m_pending_actions.push(GameActions::Action(type, playerIdentifier));
}

bool NetworkNode::PollGameAction(GameActions::Action& out)
{
	if (m_pending_actions.empty())
	{
		return false;
	}
	else
	{
		out = m_pending_actions.front();
		m_pending_actions.pop();
		return true;
	}
}

unsigned NetworkNode::GetCategory() const
{
	return Category::kNetwork;
}
