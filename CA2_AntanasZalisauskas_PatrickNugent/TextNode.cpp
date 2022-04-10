/// <summary>
/// Name: Patrick Nugent
/// Student Number: D00218208
///
/// Name: Antanas Zalisauskas
/// Student Number: D00218148
/// </summary>
#include "TextNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include "ResourceHolder.hpp"
#include "Utility.hpp"

TextNode::TextNode(const FontHolder& fonts, const std::string& text)
	: m_text(text, fonts.Get(Fonts::Main), 20)
{
}

void TextNode::SetString(const std::string& text)
{
	m_text.setString(text);
	Utility::CentreOrigin(m_text);
}

void TextNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_text, states);
}

/// <summary>
/// Written by: Antanas Zalisauskas
///
///	Changes text color
/// </summary>
/// <param name="color"></param>
void TextNode::SetColor(const sf::Color color)
{
	m_text.setFillColor(color);
}