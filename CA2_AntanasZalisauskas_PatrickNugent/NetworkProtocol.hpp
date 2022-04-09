#pragma once
#include <SFML/System/Vector2.hpp>

/// <summary>
/// Edited by: Patrick Nugent
///
/// Added overloaded constructor for Action which takes in value
/// </summary>

const unsigned short SERVER_PORT = 50000;

namespace Server
{
	//These are packets that come from the Server
	enum class PacketType
	{
		BroadcastMessage,
		InitialState,
		PlayerEvent,
		PlayerRealtimeChange,
		PlayerConnect,
		PlayerDisconnect,
		AcceptCoopPartner,
		SpawnEnemy,
		SpawnFlyingEnemy,
		SpawnPickup,
		SpawnSelf,
		UpdateClientState,
		MissionSuccess,
		UpdateGameTimeLeft,
		FinishGame,
		StartGame,
		DestroyPickup,
		StunPlayer
	};
}

namespace Client
{
	//Messages sent from the Client
	enum class PacketType
	{
		PlayerEvent,
		PlayerRealtimeChange,
		RequestCoopPartner,
		PositionUpdate,
		GameEvent,
		Quit,
		PlayerCharacterSelect,
		HostStartGame,
		Waiting,
		TimeSelection
	};
}

namespace GameActions
{
	enum Type
	{
		CollectPickup,
		StunPlayer
	};

	struct Action
	{
		Action()
		{
			
		}

		Action(Type type, sf::Vector2f position):type(type), position(position)
		{
			
		}

		Action(Type type, sf::Int16 pickupIdentifier, sf::Int8 playerIdentifier):type(type), pickupIdentifier(pickupIdentifier), playerIdentifier(playerIdentifier)
		{

		}

		Action(Type type, sf::Int8 playerIdentifier) :type(type), playerIdentifier(playerIdentifier)
		{

		}

		Type type;
		sf::Vector2f position;
		sf::Int16 pickupIdentifier;
		sf::Int8 playerIdentifier;
	};
}

