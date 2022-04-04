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
		FinishGame
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
		PlayerCharacterSelect
	};
}

namespace GameActions
{
	enum Type
	{
		EnemyExplode,
		CollectPickup
	};

	struct Action
	{
		Action()
		{
			
		}

		Action(Type type, sf::Vector2f position):type(type), position(position)
		{
			
		}

		Action(Type type, sf::Int8 value) :type(type), value(value)
		{

		}

		Type type;
		sf::Vector2f position;
		sf::Int8 value;
	};
}

