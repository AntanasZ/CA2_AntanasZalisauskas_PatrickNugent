#include "MultiplayerGameState.hpp"
#include "MusicPlayer.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>
#include <SFML/Network/Packet.hpp>

#include "PickupType.hpp"

sf::IpAddress GetAddressFromFile()
{
	{
		//Try to open existing file ip.txt
		std::ifstream input_file("ip.txt");
		std::string ip_address;
		if(input_file >> ip_address)
		{
			return ip_address;
		}
	}

	//If open/read failed, create a new file
	std::ofstream output_file("ip.txt");
	std::string local_address = "127.0.0.1";
	output_file << local_address;
	return local_address;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool is_host)
: State(stack, context)
, m_world(*context.window, *context.fonts, *context.sounds, true)
, m_window(*context.window)
, m_texture_holder(*context.textures)
, m_connected(false)
, m_game_server(nullptr)
, m_active_state(true)
, m_has_focus(true)
, m_host(is_host)
, m_game_started(false)
, m_client_timeout(sf::seconds(2.f))
, m_time_since_last_packet(sf::seconds(0.f))
{
	m_broadcast_text.setFont(context.fonts->Get(Fonts::Main));
	m_broadcast_text.setPosition(1024.f / 2, 100.f);

	m_player_invitation_text.setFont(context.fonts->Get(Fonts::Main));
	m_player_invitation_text.setCharacterSize(20);
	m_player_invitation_text.setFillColor(sf::Color::White);
	m_player_invitation_text.setString("Press Enter to spawn player 2");
	m_player_invitation_text.setPosition(1000 - m_player_invitation_text.getLocalBounds().width, 760 - m_player_invitation_text.getLocalBounds().height);

	//We reuse this text for "Attempt to connect" and "Failed to connect" messages
	m_failed_connection_text.setFont(context.fonts->Get(Fonts::Main));
	m_failed_connection_text.setString("Attempting to connect...");
	m_failed_connection_text.setCharacterSize(35);
	m_failed_connection_text.setFillColor(sf::Color::White);
	Utility::CentreOrigin(m_failed_connection_text);
	m_failed_connection_text.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

	//Render an "establishing connection" frame for user feedback
	m_window.clear(sf::Color::Black);
	m_window.draw(m_failed_connection_text);
	m_window.display();
	m_failed_connection_text.setString("Could not connect to the remote server");
	Utility::CentreOrigin(m_failed_connection_text);

	sf::IpAddress ip;
	if(m_host)
	{
		m_game_server.reset(new GameServer(sf::Vector2f(m_window.getSize())));
		ip = "127.0.0.1";
	}
	else
	{
		ip = GetAddressFromFile();
	}

	if(m_socket.connect(ip, SERVER_PORT, sf::seconds(5.f)) == sf::TcpSocket::Done)
	{
		m_connected = true;
		SendCharacterSelection();
	}
	else
	{
		m_failed_connection_clock.restart();
	}

	m_socket.setBlocking(false);

	//Play game theme
	context.music->Play(MusicThemes::kScoobyDooTheme);
}

void MultiplayerGameState::Draw()
{
	if(m_connected)
	{
		//std::cout << "Connected";
		m_world.Draw();

		//Show broadcast messages in default view
		m_window.setView(m_window.getDefaultView());

		if(!m_broadcasts.empty())
		{
			m_window.draw(m_broadcast_text);
		}

		if(m_local_player_identifiers.size() < 2 && m_player_invitation_time < sf::seconds(0.5f))
		{
			m_window.draw(m_player_invitation_text);
		}
	}
	else
	{
		m_window.draw(m_failed_connection_text);
	}
}

bool MultiplayerGameState::Update(sf::Time dt)
{
	//Connected to the Server: Handle all the network logic
	if(m_connected)
	{
		m_world.Update(dt);

		//Remove players whose aircraft were destroyed
		bool found_local_character = false;
		for(auto itr = m_players.begin(); itr != m_players.end();)
		{
			//Check if there are no more local planes for remote clients
			if(std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), itr->first) != m_local_player_identifiers.end())
			{
				found_local_character = true;
			}

			if(!m_world.GetCharacter(itr->first))
			{
				itr = m_players.erase(itr);

				//No more players left : Mission failed
				if(m_players.empty())
				{
					RequestStackPush(StateID::kGameOver);
				}
			}
			else
			{
				++itr;
			}
		}

		if(!found_local_character && m_game_started)
		{
			RequestStackPush(StateID::kGameOver);
		}

		//Only handle the realtime input if the window has focus and the game is unpaused
		if(m_active_state && m_has_focus)
		{
			CommandQueue& commands = m_world.GetCommandQueue();
			for(auto& pair : m_players)
			{
				pair.second->HandleRealtimeInput(commands);
			}
		}

		//Always handle the network input
		CommandQueue& commands = m_world.GetCommandQueue();
		for(auto& pair : m_players)
		{
			pair.second->HandleRealtimeNetworkInput(commands);
		}

		//Handle messages from the server that may have arrived
		sf::Packet packet;
		if(m_socket.receive(packet) == sf::Socket::Done)
		{
			m_time_since_last_packet = sf::seconds(0.f);
			sf::Int32 packet_type;
			packet >> packet_type;
			HandlePacket(packet_type, packet);
		}
		else
		{
			//Check for timeout with the server
			if(m_time_since_last_packet > m_client_timeout)
			{
				m_connected = false;
				m_failed_connection_text.setString("Lost connection to the server");
				Utility::CentreOrigin(m_failed_connection_text);

				m_failed_connection_clock.restart();
			}
		}

		UpdateBroadcastMessage(dt);

		//Time counter fro blinking second player text
		m_player_invitation_time += dt;
		if(m_player_invitation_time > sf::seconds(1.f))
		{
			m_player_invitation_time = sf::Time::Zero;
		}

		//Events occurring in the game
		GameActions::Action game_action;
		while(m_world.PollGameAction(game_action))
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::GameEvent);
			packet << static_cast<sf::Int32>(game_action.type);
			packet << game_action.position.x;
			packet << game_action.position.y;

			m_socket.send(packet);
		}

		//Regular position updates
		if(m_tick_clock.getElapsedTime() > sf::seconds(1.f/20.f))
		{
			sf::Packet position_update_packet;
			position_update_packet << static_cast<sf::Int32>(Client::PacketType::PositionUpdate);
			position_update_packet << static_cast<sf::Int32>(m_local_player_identifiers.size());

			for(sf::Int32 identifier : m_local_player_identifiers)
			{
				if(Character* character = m_world.GetCharacter(identifier))
				{
					//position_update_packet << identifier << character->getPosition().x << character->getPosition().y << static_cast<sf::Int32>(character->GetType());
					position_update_packet << identifier << character->getPosition().x << character->getPosition().y;
				}
			}
			m_socket.send(position_update_packet);
			m_tick_clock.restart();
		}
		m_time_since_last_packet += dt;
	}

	//Failed to connect and waited for more than 5 seconds: Back to menu
	else if(m_failed_connection_clock.getElapsedTime() >= sf::seconds(5.f))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return true;
}

bool MultiplayerGameState::HandleEvent(const sf::Event& event)
{
	//Game input handling
	CommandQueue& commands = m_world.GetCommandQueue();

	//Forward events to all players
	for(auto& pair : m_players)
	{
		pair.second->HandleEvent(event, commands);
	}

	if(event.type == sf::Event::KeyPressed)
	{
		//If enter pressed, add second player co-op only if there is only 1 player
		if(event.key.code == sf::Keyboard::Return && m_local_player_identifiers.size()==1)
		{
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::RequestCoopPartner);
			m_socket.send(packet);
		}
		//If escape is pressed, show the pause screen
		else if(event.key.code == sf::Keyboard::Escape)
		{
			DisableAllRealtimeActions();
			RequestStackPush(StateID::kNetworkPause);
		}
	}
	else if(event.type == sf::Event::GainedFocus)
	{
		m_has_focus = true;
	}
	else if(event.type == sf::Event::LostFocus)
	{
		m_has_focus = false;
	}
	return true;
}

void MultiplayerGameState::OnActivate()
{
	m_active_state = true;
}

void MultiplayerGameState::OnDestroy()
{
	if(!m_host && m_connected)
	{
		//Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<sf::Int32>(Client::PacketType::Quit);
		m_socket.send(packet);
	}
}

void MultiplayerGameState::SendCharacterSelection()
{
	//Inform server of this client's character choice
	sf::Packet packet;
	packet << static_cast<sf::Int32>(Client::PacketType::PlayerCharacterSelect);

	for (sf::Int32 identifier : m_local_player_identifiers)
	{
		if (Character* character = m_world.GetCharacter(identifier))
		{
			packet << identifier << DetermineNumberFromCharacter(character->GetType());
		}
	}

	m_socket.send(packet);
}

void MultiplayerGameState::DisableAllRealtimeActions()
{
	m_active_state = false;
	for(sf::Int32 identifier : m_local_player_identifiers)
	{
		m_players[identifier]->DisableAllRealtimeActions();
	}
}

void MultiplayerGameState::UpdateBroadcastMessage(sf::Time elapsed_time)
{
	if(m_broadcasts.empty())
	{
		return;
	}

	//Update broadcast timer
	m_broadcast_elapsed_time += elapsed_time;
	if(m_broadcast_elapsed_time > sf::seconds(2.f))
	{
		//If message has expired, remove it
		m_broadcasts.erase(m_broadcasts.begin());

		//Continue to display the next broadcast message
		if(!m_broadcasts.empty())
		{
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
	}
}

CharacterType MultiplayerGameState::DetermineCharacterFromNumber(sf::Int8 characterNumber)
{
	if (characterNumber == 1)
	{
		return CharacterType::kScooby;
	}
	else if (characterNumber == 2)
	{
		return CharacterType::kShaggy;
	}
	else if (characterNumber == 3)
	{
		return CharacterType::kFred;
	}
	else if (characterNumber == 4)
	{
		return CharacterType::kVelma;
	}
	else if (characterNumber == 2)
	{
		return CharacterType::kDaphne;
	}
	else
	{
		return CharacterType::kShaggy;
	}
}

sf::Int32 MultiplayerGameState::DetermineNumberFromCharacter(CharacterType characterType)
{
	if (characterType == CharacterType::kScooby)
	{
		return 1;
	}
	else if (characterType == CharacterType::kShaggy)
	{
		return 2;
	}
	else if (characterType == CharacterType::kFred)
	{
		return 3;
	}
	else if (characterType == CharacterType::kVelma)
	{
		return 4;
	}
	else if (characterType == CharacterType::kDaphne)
	{
		return 5;
	}
	else
	{
		return 2;
	}
}

void MultiplayerGameState::HandlePacket(sf::Int32 packet_type, sf::Packet& packet)
{
	switch (static_cast<Server::PacketType>(packet_type))
	{
		//Send message to all Clients
	case Server::PacketType::BroadcastMessage:
	{
		std::string message;
		packet >> message;
		m_broadcasts.push_back(message);

		//Just added the first message, display immediately
		if (m_broadcasts.size() == 1)
		{
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
	}
	break;

	//Sent by the server to spawn player 1 character on connect
	case Server::PacketType::SpawnSelf:
	{
		sf::Int32 character_identifier;
		sf::Vector2f character_position;
		sf::Int8 character_type;
		packet >> character_identifier >> character_position.x >> character_position.y >> character_type;
		Character* character = m_world.AddCharacter(character_identifier, *GetContext().characterSelection);
		character->setPosition(character_position);
		m_players[character_identifier].reset(new Player(&m_socket, character_identifier, GetContext().keys1));
		m_local_player_identifiers.push_back(character_identifier);
		m_game_started = true;
	}
	break;

	case Server::PacketType::PlayerConnect:
	{
		sf::Int32 character_identifier;
		sf::Vector2f character_position;
		sf::Int8 character_type;
		packet >> character_identifier >> character_position.x >> character_position.y >> character_type;
		
		Character* character = m_world.AddCharacter(character_identifier, DetermineCharacterFromNumber(character_type));
		character->setPosition(character_position);
		m_players[character_identifier].reset(new Player(&m_socket, character_identifier, nullptr));
	}
	break;

	case Server::PacketType::PlayerDisconnect:
	{
		sf::Int32 character_identifier;
		packet >> character_identifier;
		m_world.RemoveCharacter(character_identifier);
		m_players.erase(character_identifier);
	}
	break;

	case Server::PacketType::InitialState:
	{
		sf::Int32 character_count;
		float world_height, current_scroll;
		packet >> world_height >> current_scroll;

		m_world.SetWorldHeight(world_height);

		packet >> character_count;
		for (sf::Int32 i = 0; i < character_count; ++i)
		{
			sf::Int32 character_identifier;
			sf::Int32 hitpoints;
			sf::Int8 character_type;
			sf::Vector2f character_position;
			packet >> character_identifier >> character_position.x >> character_position.y >> hitpoints >> character_type;

			Character* character = m_world.AddCharacter(character_identifier, DetermineCharacterFromNumber(character_type));
			character->setPosition(character_position);
			character->SetHitpoints(hitpoints);
			character->SetType(DetermineCharacterFromNumber(character_type));

			m_players[character_identifier].reset(new Player(&m_socket, character_identifier, nullptr));
		}
	}
	break;

	case Server::PacketType::AcceptCoopPartner:
	{
		/*sf::Int32 character_identifier;
		packet >> character_identifier;

		m_world.AddCharacter(character_identifier, CharacterType::kScooby);
		m_players[character_identifier].reset(new Player(&m_socket, character_identifier, GetContext().keys2));
		m_local_player_identifiers.emplace_back(character_identifier);*/
	}
	break;

	//Player event, like missile fired occurs
	case Server::PacketType::PlayerEvent:
	{
		sf::Int32 aircraft_identifier;
		sf::Int32 action;
		packet >> aircraft_identifier >> action;

		auto itr = m_players.find(aircraft_identifier);
		if (itr != m_players.end())
		{
			itr->second->HandleNetworkEvent(static_cast<PlayerAction>(action), m_world.GetCommandQueue());
		}
	}
	break;

	//Player's movement or fire keyboard state changes
	case Server::PacketType::PlayerRealtimeChange:
	{
		sf::Int32 character_identifier;
		sf::Int32 action;
		bool action_enabled;
		packet >> character_identifier >> action >> action_enabled;

		auto itr = m_players.find(character_identifier);
		if (itr != m_players.end())
		{
			itr->second->HandleNetworkRealtimeChange(static_cast<PlayerAction>(action), action_enabled);
		}
	}
	break;

	//New Enemy to be created
	case Server::PacketType::SpawnEnemy:
	{
		m_world.SpawnEnemies();
		/*float height;
		sf::Int32 type;
		float relative_x;
		packet >> type >> height >> relative_x;*/

		//m_world.AddEnemy(static_cast<CharacterType>(type), false, relative_x, height);
		//m_world.SortEnemies();
	}
	break;

	case Server::PacketType::SpawnFlyingEnemy:
	{
		m_world.SpawnFlyingEnemies();
	}
	break;

	//Mission Successfully completed
	case Server::PacketType::MissionSuccess:
	{
		RequestStackPush(StateID::kMissionSuccess);
	}
	break;

	case Server::PacketType::UpdateGameTimeLeft:
	{
		float remaining_time;
		packet >> remaining_time;
		
		m_world.DisplayRemainingGameTime(remaining_time);
	}
	break;

	case Server::PacketType::FinishGame:
	{
		RequestStackPush(StateID::kGameOver);
	}

	//Pickup created
	case Server::PacketType::SpawnPickup:
	{
		m_world.SpawnPickups();
		/*sf::Int32 type;
		sf::Vector2f position;
		packet >> type >> position.x >> position.y;
		std::cout << "Spawning pickup type " << type << std::endl;*/
		//m_world.AddPickup(position, static_cast<PickupType>(type));
	}
	break;

	case Server::PacketType::UpdateClientState:
	{
		float current_world_position;
		sf::Int32 character_count;
		packet >> current_world_position >> character_count;

		float current_view_position = m_world.GetViewBounds().top + m_world.GetViewBounds().height;

		//Set the world's scroll compensation according to whether the view is behind or ahead
		m_world.SetWorldScrollCompensation(current_view_position / current_world_position);

		for (sf::Int32 i = 0; i < character_count; ++i)
		{
			sf::Vector2f character_position;
			sf::Int32 character_identifier;
			sf::Int32 hitpoints;
			sf::Int8 character_type;
			packet >> character_identifier >> character_position.x >> character_position.y >> hitpoints >> character_type;

			Character* character = m_world.GetCharacter(character_identifier);
			bool is_local_character = std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), character_identifier) != m_local_player_identifiers.end();
			if(character && !is_local_character)
			{
				sf::Vector2f interpolated_position = character->getPosition() + (character_position - character->getPosition()) * 0.1f;
				character->setPosition(interpolated_position);
				character->SetHitpoints(hitpoints);
				character->SetType(DetermineCharacterFromNumber(character_type));
			}
		}
	}
	break;
	}
}
