/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>

#pragma once
#include "Command.hpp"
#include "KeyBinding.hpp"
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Window/Event.hpp>
#include <map>
#include "CommandQueue.hpp"
#include "PlayerAction.hpp"

class Player
{
public:
	Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding);
	void HandleEvent(const sf::Event& event, CommandQueue& commands);
	void HandleRealtimeInput(CommandQueue& commands);
	void HandleRealtimeNetworkInput(CommandQueue& commands);

	//React to events or realtime state changes recevied over the network
	void HandleNetworkEvent(PlayerAction action, CommandQueue& commands);
	void HandleNetworkRealtimeChange(PlayerAction action, bool action_enabled);

	void AssignKey(PlayerAction action, sf::Keyboard::Key key);
	sf::Keyboard::Key GetAssignedKey(PlayerAction action) const;

	void DisableAllRealtimeActions();
	bool IsLocal() const;

private:
	void InitialiseActions();
	static bool IsRealtimeAction(PlayerAction action);

private:
	const KeyBinding* m_key_binding;
	std::map<PlayerAction, Command> m_action_binding;
	std::map<PlayerAction, bool> m_action_proxies;
	int m_identifier;
	sf::TcpSocket* m_socket;
};

