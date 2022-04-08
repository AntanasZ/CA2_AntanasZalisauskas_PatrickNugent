#pragma once
#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "GameServer.hpp"
#include "NetworkProtocol.hpp"
#include "Container.hpp"

class MultiplayerGameState : public State
{
public:
	MultiplayerGameState(StateStack& stack, Context context, bool is_host);
	virtual void Draw();
	virtual bool Update(sf::Time dt);
	virtual bool HandleEvent(const sf::Event& event);
	virtual void OnActivate();
	virtual CharacterType DetermineCharacterFromNumber(int characterNumber);
	virtual int DetermineNumberFromCharacter(CharacterType characterType);
	void OnDestroy();
	void DisplayPlayers(sf::Int8 connectedPlayers);
	void SendCharacterSelection();
	void DisableAllRealtimeActions();

private:
	void UpdateBroadcastMessage(sf::Time elapsed_time);
	void HandlePacket(sf::Int32 packet_type, sf::Packet& packet);

private:
	typedef std::unique_ptr<Player> PlayerPtr;

private:
	World m_world;
	sf::RenderWindow& m_window;
	TextureHolder& m_texture_holder;

	std::map<int, PlayerPtr> m_players;
	std::vector<sf::Int32> m_local_player_identifiers;
	sf::TcpSocket m_socket;
	bool m_connected;
	std::unique_ptr<GameServer> m_game_server;
	sf::Clock m_tick_clock;

	std::vector<std::string> m_broadcasts;
	sf::Text m_broadcast_text;
	sf::Time m_broadcast_elapsed_time;

	sf::Text m_failed_connection_text;
	sf::Clock m_failed_connection_clock;

	sf::Text m_waiting_text;
	sf::Text m_time_selection_text;
	sf::Text m_players_connected_text;

	bool m_active_state;
	bool m_has_focus;
	bool m_host;
	bool m_game_started;
	sf::Time m_client_timeout;
	sf::Time m_time_since_last_packet;
	GUI::Container m_gui_container;
};

