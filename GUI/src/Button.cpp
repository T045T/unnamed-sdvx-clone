#include "stdafx.h"
#include "Button.hpp"
#include "GUIRenderer.hpp"

Button::Button(std::shared_ptr<CommonGUIStyle> style)
{
	m_style = std::move(style);
}

void Button::SetText(const WString& text)
{
	if (m_textString != text)
	{
		m_textString = text;
		m_dirty = true;
	}
}

uint32 Button::GetFontSize() const
{
	return m_fontSize;
}

void Button::SetFontSize(uint32 size)
{
	if (size != m_fontSize)
	{
		m_fontSize = size;
		m_dirty = true;
	}
}

void Button::PreRender(GUIRenderData rd, GUIElementBase*& inputElement)
{
	if (m_dirty)
	{
		m_text = rd.guiRenderer->font->create_text(m_textString, m_fontSize);
	}

	m_cachedInnerRect = m_style->buttonBorder.Apply(rd.area);
	m_hovered = rd.OverlapTest(m_cachedInnerRect);

	if (!m_animation)
	{
		if (m_hovered || m_held)
		{
			AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&m_animationPadding, -2.0f, 0.1f)), true);
			m_animation = true;
		}
	}
	else
	{
		if (!m_hovered && !m_held)
		{
			AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&m_animationPadding, 0.0f, 0.2f)), true);
			m_animation = false;
		}
	}

	if (m_hovered)
		inputElement = this;
}

void Button::Render(GUIRenderData rd)
{
	m_TickAnimations(rd.deltaTime);

	// Render BG
	Margini padding = static_cast<int32>(m_animationPadding);
	rd.area = padding.Apply(rd.area);
	rd.guiRenderer->RenderButton(rd.area, (m_hovered) ? m_style->buttonHighlightTexture : m_style->buttonTexture,
		m_style->buttonBorder);

	if (m_hovered && rd.guiRenderer->GetMouseButtonPressed(MouseButton::Left))
	{
		m_held = true;
		rd.guiRenderer->SetInputFocus(nullptr);
		OnPressed.Call();
	}

	if (m_held)
	{
		if (!rd.guiRenderer->GetMouseButton(MouseButton::Left))
			m_held = false;
		else
			rd.area.pos += Vector2(1.0f);
	}

	// Render content(Text) but clipped to button insides
	rd.guiRenderer->PushScissorRect(m_cachedInnerRect);

	// Render text
	const Rect textRect = GUISlotBase::ApplyAlignment(Vector2(0.5f), Rect(Vector2(), m_text->size), rd.area);
	const Color color = Color::White;
	rd.guiRenderer->RenderText(m_text, textRect.pos, color);

	rd.guiRenderer->PopScissorRect();
}

Vector2 Button::GetDesiredSize(GUIRenderData rd)
{
	if (m_dirty)
		m_text = rd.guiRenderer->font->create_text(m_textString, m_fontSize);

	Vector2 sizeOut;

	if (m_text)
	{
		sizeOut = m_text->size;
		sizeOut += m_style->buttonBorder.GetSize() + m_style->buttonPadding.GetSize();
	}
	return sizeOut;
}
