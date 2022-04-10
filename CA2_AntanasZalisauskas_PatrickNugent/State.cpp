/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "State.hpp"

#include "StateStack.hpp"

/// <summary>
/// Edited by: Patrick Nugent
///
///	Added fields for storing character and mode selection
/// </summary>
State::Context::Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, MusicPlayer& music, SoundPlayer& sounds, KeyBinding& keys1, KeyBinding& keys2, CharacterType& characterSelection, StateID& modeSelection)
: window(&window)
, textures(&textures)
, fonts(&fonts)
, music(&music)
, sounds(&sounds)
, keys1(&keys1)
, keys2(&keys2)
, characterSelection(&characterSelection)
, modeSelection(&modeSelection)
{
}

State::State(StateStack& stack, Context context)
:m_stack(&stack)
, m_context(context)
{
}

State::~State()
{
}


void State::RequestStackPush(StateID state_id)
{
	m_stack->PushState(state_id);
}

void State::RequestStackPop()
{
	m_stack->PopState();
}

void State::RequestStackClear()
{
	m_stack->ClearStates();
}

State::Context State::GetContext() const
{
	return m_context;
}

void State::OnActivate()
{

}

void State::OnDestroy()
{

}
