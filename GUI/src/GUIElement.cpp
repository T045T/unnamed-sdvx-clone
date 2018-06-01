#include "stdafx.h"
#include "GUI.hpp"
#include "GUIRenderer.hpp"

GUIElementBase::~GUIElementBase()
{
	if (m_rendererFocus)
	{
		m_rendererFocus->SetInputFocus(nullptr);
	}
}

void GUIElementBase::PreRender(GUIRenderData rd, GUIElementBase*& inputElement)
{}

Vector2 GUIElementBase::GetDesiredSize(GUIRenderData rd)
{
	return Vector2();
}

bool GUIElementBase::AddAnimation(std::shared_ptr<IGUIAnimation> anim, bool removeOld)
{
	void* target = anim->GetTarget();
	if (m_animationMap.Contains(target))
	{
		if (!removeOld)
			return false;
		m_animationMap.erase(target);
	}

	m_animationMap.Add(target, std::move(anim));
	return true;
}

std::shared_ptr<IGUIAnimation> GUIElementBase::GetAnimation(uint32 uid)
{
	const auto suid = static_cast<size_t>(uid);
	return GetAnimation(reinterpret_cast<void*>(suid));
}

std::shared_ptr<IGUIAnimation> GUIElementBase::GetAnimation(void* target)
{
	const auto found = m_animationMap.Find(target);
	if (found)
		return *found;
	return std::shared_ptr<IGUIAnimation>();
}

bool GUIElementBase::HasInputFocus() const
{
	return m_rendererFocus != nullptr;
}

bool GUIElementBase::OverlapTest(Rect rect, Vector2 point)
{
	if (point.x < rect.Left() || point.x > rect.Right())
		return false;
	if (point.y < rect.Top() || point.y > rect.Bottom())
		return false;
	return true;
}

void GUIElementBase::m_OnRemovedFromParent()
{
	slot = nullptr;
}

void GUIElementBase::m_AddedToSlot(GUISlotBase* slot)
{}

void GUIElementBase::m_OnZOrderChanged(GUISlotBase* slot)
{}

void GUIElementBase::m_TickAnimations(float deltaTime)
{
	for (auto it = m_animationMap.begin(); it != m_animationMap.end();)
	{
		const bool done = !it->second->Update(deltaTime);
		if (done)
		{
			it = m_animationMap.erase(it);
			continue;
		}
		++it;
	}
}

GUISlotBase::~GUISlotBase()
{
	if (element)
		element->m_OnRemovedFromParent();
}

void GUISlotBase::PreRender(GUIRenderData rd, GUIElementBase*& inputElement)
{
	// Apply padding
	rd.area = padding.Apply(rd.area);

	// Store area
	m_cachedArea = rd.area;

	element->PreRender(rd, inputElement);
}

void GUISlotBase::Render(GUIRenderData rd)
{
	// Scissor Rectangle
	rd.area = m_cachedArea;

	rd.guiRenderer->PushScissorRect(rd.area);
	element->Render(rd);
	rd.guiRenderer->PopScissorRect();
}

Vector2 GUISlotBase::GetDesiredSize(const GUIRenderData rd)
{
	const Vector2 size = element->GetDesiredSize(rd);
	return size + padding.GetSize();
}

Rect GUISlotBase::ApplyFill(FillMode fillMode, const Vector2& inSize, const Rect& rect)
{
	if (fillMode == FillMode::None)
	{
		return Rect(rect.pos, inSize);
	}
	else if (fillMode == FillMode::Stretch)
	{
		return rect;
	}
	else if (fillMode == FillMode::Fit)
	{
		const float rx = inSize.x / rect.size.x;
		const float ry = inSize.y / rect.size.y;
		float scale = 1.0f;

		if (rx > ry)
			scale = 1.0f / rx;
		else // ry is largest
			scale = 1.0f / ry;

		Rect ret = rect;
		{
			const Vector2 newSize = inSize * scale;
			ret.size = newSize;
		}
		return ret;
	}
	else // Fill
	{
		const float rx = inSize.x / rect.size.x;
		const float ry = inSize.y / rect.size.y;
		float scale = 1.0f;

		if (rx < ry)
			scale = 1.0f / rx;
		else // ry is smallest
			scale = 1.0f / ry;

		Rect ret = rect;
		{
			const Vector2 newSize = inSize * scale;
			ret.size = newSize;
		}

		return ret;
	}
}

Rect GUISlotBase::ApplyAlignment(const Vector2& alignment, const Rect& rect, const Rect& parent)
{
	Vector2 remaining = parent.size - rect.size;
	remaining.x = remaining.x;
	remaining.y = remaining.y;

	return Rect(parent.pos + remaining * alignment, rect.size);
}

void GUISlotBase::SetZOrder(int32 zorder)
{
	m_zorder = zorder;
	parent->m_OnZOrderChanged(this);
}

int32 GUISlotBase::GetZOrder() const
{
	return m_zorder;
}
