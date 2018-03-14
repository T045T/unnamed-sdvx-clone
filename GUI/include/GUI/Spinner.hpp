#pragma once
#include "GUIElement.hpp"
#include "CommonGUIStyle.hpp"

/*
	Spinning image used for loading images
*/
class Spinner : public GUIElementBase
{
public:
	Spinner(std::shared_ptr<CommonGUIStyle> style);
	virtual void Render(GUIRenderData rd) override;
	virtual Vector2 GetDesiredSize(GUIRenderData rd) override;

private:
	float m_rotation = 0.0f;
	std::shared_ptr<CommonGUIStyle> m_style;
};