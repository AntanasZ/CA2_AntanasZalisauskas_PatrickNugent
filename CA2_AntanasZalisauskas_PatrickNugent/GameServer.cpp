#include "GameServer.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/System.hpp>

#include <SFML/Network/Packet.hpp>

#include "PickupType.hpp"
#include "Utility.hpp"
#include <iostream>

//It is essential to set the sockets to non-blocking - m_socket.setBlocking(false)
//otherwise the server will hang waiting to read input from a connection

GameServer::RemotePeer::RemotePeer():m_ready(false), m_timed_out(false)
{
	m_socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f battlefield_size)
	: m_thread(&GameServer::ExecutionThread, this)
	, m_listening_state(false)
	, m_client_timeout(sf::seconds(1.f))
	, m_max_connected_players(15)
	, m_connected_players(0)
	, m_world_height(5000.f)
	, m_battlefield_rect(0.f, m_world_height-battlefield_size.y, battlefield_size.x, battlefield_size.y)
	, m_battlefield_scrollspeed(-50.f)
	, m_character_count(0)
	, m_peers(1)
	, m_character_identifier_counter(1)
	, m_waiting_thread_end(false)
	, m_last_spawn_time(sf::Time::Zero)
	, m_time_for_next_spawn(sf::seconds(5.f))
	, m_enemy_spawn_countdown()
	, m_flying_enemy_spawn_countdown()
	, m_pickup_spawn_countdown()
	//, m_game_countdown(sf::seconds(30))
	, m_game_countdown(sf::seconds(1800))
	, m_game_over(false)
	, m_game_started(false)
{
	m_listener_socket.setBlocking(false);
	m_peers[0].reset(new RemotePeer());
	m_thread.launch();
}

GameServer::~GameServer()
{
	m_waiting_thread_end = true;
	m_thread.wait();
}

//This is the same as SpawnSelf but indicate that an aircraft from a different client is entering the world

void GameServer::NotifyPlayerSpawn(sf::Int32 character_identifier)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<sf::Int32>(Server::PacketType::PlayerConnect);
	packet << character_identifier << m_character_info[character_identifier].m_position.x << m_character_info[character_identifier].m_position.y << m_character_info[character_identifier].m_type;
	for(std::size_t i=0; i < m_connected_players; ++i)
	{
		if(m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

//This is the same as PlayerEvent, but for real-time actions. This means that we are changing an ongoing state to either true or false, so we add a Boolean value to the parameters

void GameServer::NotifyPlayerRealtimeChange(sf::Int32 character_identifier, sf::Int32 action, bool action_enabled)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<sf::Int32>(Server::PacketType::PlayerRealtimeChange);
	packet << character_identifier;
	packet << action;
	packet << action_enabled;

	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

//This takes two sf::Int32 variables, the aircraft identifier and the action identifier
//as declared in the Player class. This is used to inform all peers that plane X has
//triggered an action

void GameServer::NotifyPlayerEvent(sf::Int32 character_identifier, sf::Int32 action)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<sf::Int32>(Server::PacketType::PlayerEvent);
	packet << character_identifier;
	packet << action;

	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

void GameServer::SetListening(bool enable)
{
	//Check if the server listening socket is already listening
	if (enable)
	{
		if (!m_listening_state)
		{
			m_listening_state = (m_listener_socket.listen(SERVER_PORT) == sf::TcpListener::Done);
		}
	}
	else
	{
		m_listener_socket.close();
		m_listening_state = false;
	}
}


void GameServer::ExecutionThread()
{
	SetListening(true);

	sf::Time frame_rate = sf::seconds(1.f / 60.f);
	sf::Time frame_time = sf::Time::Zero;
	sf::Time tick_rate = sf::seconds(1.f / 20.f);
	sf::Time tick_time = sf::Time::Zero;
	sf::Clock frame_clock, tick_clock;

	srand(time(NULL));

	while(!m_waiting_thread_end)
	{
		HandleIncomingConnections();
		HandleIncomingPackets();

		frame_time += frame_clock.getElapsedTime();
		frame_clock.restart();

		tick_time += tick_clock.getElapsedTime();
		tick_clock.restart();

		//Fixed update step
		while(frame_time >= frame_rate)
		{
			//m_battlefield_rect.top += m_battlefield_scrollspeed * frame_rate.asSeconds();
			frame_time -= frame_rate;
		}

		//Fixed tick step
		while(tick_time >= tick_rate)
		{
			Tick(tick_time);
			tick_time -= tick_rate;
		}

		//sleep
		sf::sleep(sf::milliseconds(100));

	}
}

void GameServer::Tick(sf::Time tick_time)
{
	UpdateClientState();

	if (m_game_started)
	{
		m_enemy_spawn_countdown += tick_time;
		m_flying_enemy_spawn_countdown += tick_time;
		m_pickup_spawn_countdown += tick_time;

		if (m_game_countdown >= sf::Time::Zero && !m_game_over)
		{
			m_game_countdown -= tick_time;

			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::UpdateGameTimeLeft) << m_game_countdown.asSeconds();

			SendToAll(packet);
		}
		else
		{
			m_game_over = true;

			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::FinishGame);

			SendToAll(packet);
		}

		if (!m_game_over)
		{
			if (m_enemy_spawn_countdown >= sf::seconds(10.f))
			{
				sf::Packet packet;
				sf::Int8 randomEnemy;
				randomEnemy = rand() % 12;
				packet << static_cast<sf::Int32>(Server::PacketType::SpawnEnemy) << randomEnemy;

				SendToAll(packet);

				m_enemy_spawn_countdown = sf::Time::Zero;
			}

			if (m_flying_enemy_spawn_countdown >= sf::seconds(15.f))
			{
				sf::Packet packet;
				sf::Int8 randomEnemy;
				randomEnemy = rand() % 12;
				packet << static_cast<sf::Int32>(Server::PacketType::SpawnFlyingEnemy) << randomEnemy;

				SendToAll(packet);

				m_flying_enemy_spawn_countdown = sf::Time::Zero;
			}

			if (m_pickup_spawn_countdown >= sf::seconds(2.f))
			{
				sf::Packet packet;
				sf::Int8 randomPickup;
				sf::Int16 randomPosition;
				randomPickup = rand() % 9;
				randomPosition = (rand() % 2950) + 90;
				packet << static_cast<sf::Int32>(Server::PacketType::SpawnPickup) << randomPickup << randomPosition;

				SendToAll(packet);

				m_pickup_spawn_countdown = sf::Time::Zero;
			}
		}
	}
}

sf::Time GameServer::Now() const
{
	return m_clock.getElapsedTime();
}

void GameServer::HandleIncomingPackets()
{
	bool detected_timeout = false;

	for(PeerPtr& peer : m_peers)
	{
		if(peer->m_ready)
		{
			sf::Packet packet;
			while(peer->m_socket.receive(packet) == sf::Socket::Done)
			{
				//Interpret the packet and react to it
				HandleIncomingPacket(packet, *peer, detected_timeout);

				peer->m_last_packet_time = Now();
				packet.clear();
			}

			if(Now() > peer->m_last_packet_time + m_client_timeout)
			{
				peer->m_timed_out = true;
				detected_timeout = true;
			}

		}
	}

	if(detected_timeout)
	{
		HandleDisconnections();
	}

}

void GameServer::HandleIncomingPacket(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout)
{
	sf::Int32 packet_type;
	packet >> packet_type;

	switch (static_cast<Client::PacketType> (packet_type))
	{
	case Client::PacketType::Quit:
	{
		receiving_peer.m_timed_out = true;
		detected_timeout = true;
	}
	break;

	case Client::PacketType::PlayerEvent:
	{
		sf::Int32 character_identifier;
		sf::Int32 action;
		packet >> character_identifier >> action;
		NotifyPlayerEvent(character_identifier, action);
	}
	break;

	case Client::PacketType::PlayerRealtimeChange:
	{
		sf::Int32 character_identifier;
		sf::Int32 action;
		bool action_enabled;
		packet >> character_identifier >> action >> action_enabled;
		NotifyPlayerRealtimeChange(character_identifier, action, action_enabled);
	}
	break;

	case Client::PacketType::PlayerCharacterSelect:
	{
		sf::Int32 character_identifier;
		sf::Int8 character_type;
		packet >> character_identifier >> character_type;
		m_character_info[character_identifier].m_type = character_type;
	}
	break;

	case Client::PacketType::PositionUpdate:
	{
		sf::Int32 num_characters;
		packet >> num_characters;

		for (sf::Int32 i = 0; i < num_characters; ++i)
		{
			sf::Int32 character_identifier;
			sf::Int16 character_score;
			sf::Vector2f character_position;
			packet >> character_identifier >> character_position.x >> character_position.y >> character_score;
			m_character_info[character_identifier].m_position = character_position;
			m_character_info[character_identifier].m_score = character_score;
		}
	}
	break;

	case Client::PacketType::GameEvent:
	{
		sf::Int32 action;
		float x;
		float y;

		packet >> action;
		packet >> x;
		packet >> y;

		//Enemy explodes, with a certain probability, drop a pickup
		//To avoid multiple messages only listen to the first peer (host)
		if (action == GameActions::EnemyExplode && Utility::RandomInt(3) == 0 && &receiving_peer == m_peers[0].get())
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Server::PacketType::SpawnPickup);
			packet << static_cast<sf::Int32>(Utility::RandomInt(static_cast<int>(PickupType::kPickupCount)));
			packet << x;
			packet << y;

			SendToAll(packet);
		}
	}

	case Client::PacketType::HostStartGame:
	{
		m_game_started = true;
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::PacketType::StartGame);

		SendToAll(packet);
	}
	break;
	}

}

void GameServer::HandleIncomingConnections()
{
	if(!m_listening_state)
	{
		return;
	}

	if(m_listener_socket.accept(m_peers[m_connected_players]->m_socket) == sf::TcpListener::Done)
	{
		//Order the new client to spawn its player 1
		m_character_info[m_character_identifier_counter].m_position = sf::Vector2f(m_battlefield_rect.width / 2, m_battlefield_rect.top + m_battlefield_rect.height / 2);
		m_character_info[m_character_identifier_counter].m_score = 0;
		m_character_info[m_character_identifier_counter].m_type = 6;

		sf::Packet packet;
		packet << static_cast<sf::Int32>(Server::PacketType::SpawnSelf);
		packet << m_character_identifier_counter;
		packet << m_character_info[m_character_identifier_counter].m_position.x;
		packet << m_character_info[m_character_identifier_counter].m_position.y;

		m_peers[m_connected_players]->m_character_identifiers.emplace_back(m_character_identifier_counter);

		BroadcastMessage("New player");
		InformWorldState(m_peers[m_connected_players]->m_socket);
		NotifyPlayerSpawn(m_character_identifier_counter++);

		m_peers[m_connected_players]->m_socket.send(packet);
		m_peers[m_connected_players]->m_ready = true;
		m_peers[m_connected_players]->m_last_packet_time = Now();

		m_character_count++;
		m_connected_players++;

		if(m_connected_players >= m_max_connected_players)
		{
			SetListening(false);
		}
		else
		{
			m_peers.emplace_back(PeerPtr(new RemotePeer()));
		}
	}
}

void GameServer::HandleDisconnections()
{
	for(auto itr = m_peers.begin(); itr != m_peers.end();)
	{
		if((*itr)->m_timed_out)
		{
			//Inform everyone of a disconnection, erase
			for(sf::Int32 identifer : (*itr)->m_character_identifiers)
			{
				SendToAll((sf::Packet() << static_cast<sf::Int32>(Server::PacketType::PlayerDisconnect) << identifer));
				m_character_info.erase(identifer);
			}

			m_connected_players--;
			m_character_count -= (*itr)->m_character_identifiers.size();

			itr = m_peers.erase(itr);

			//If the number of peers has dropped below max_connections
			if(m_connected_players < m_max_connected_players)
			{
				m_peers.emplace_back(PeerPtr(new RemotePeer()));
				SetListening(true);
			}

			BroadcastMessage("A player has disconnected");

		}
		else
		{
			++itr;
		}
	}
}

void GameServer::InformWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::PacketType::InitialState);
	packet << m_world_height << m_battlefield_rect.top + m_battlefield_rect.height;
	packet << static_cast<sf::Int32>(m_character_count);

	for(std::size_t i=0; i < m_connected_players; ++i)
	{
		if(m_peers[i]->m_ready)
		{
			for(sf::Int32 identifier : m_peers[i]->m_character_identifiers)
			{
				packet << identifier << m_character_info[identifier].m_position.x << m_character_info[identifier].m_position.y << m_character_info[identifier].m_score << m_character_info[identifier].m_type;
			}
		}
	}

	socket.send(packet);
}

void GameServer::BroadcastMessage(const std::string& message)
{
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Server::PacketType::BroadcastMessage);
	packet << message;
	for(std::size_t i=0; i < m_connected_players; ++i)
	{
		if(m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

void GameServer::SendToAll(sf::Packet& packet)
{
	for(PeerPtr& peer : m_peers)
	{
		if(peer->m_ready)
		{
			peer->m_socket.send(packet);
		}
	}
}

void GameServer::UpdateClientState()
{
	sf::Packet update_client_state_packet;
	update_client_state_packet << static_cast<sf::Int32>(Server::PacketType::UpdateClientState);
	update_client_state_packet << static_cast<float>(m_battlefield_rect.top + m_battlefield_rect.height);
	update_client_state_packet << static_cast<sf::Int32>(m_character_count);

	for(const auto& character : m_character_info)
	{
		update_client_state_packet << character.first << character.second.m_position.x << character.second.m_position.y << character.second.m_score << character.second.m_type;
	}

	SendToAll(update_client_state_packet);
}
