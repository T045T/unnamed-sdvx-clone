#pragma once

/* 
	Game background base class
*/
class Background
{
public:
	Background(class Game* game, bool foreground = false);
	~Background() = default;

	void Render(float deltaTime);

private:
	Game* game;
	RenderState renderState;
	Mesh fullscreenMesh;
	Material fullscreenMaterial;
	Texture backgroundTexture;
	MaterialParameterSet fullscreenMaterialParams;
	float clearTransition = 0.0f;

	Material LoadBackgroundMaterial(const String& path) const;
	Texture LoadBackgroundTexture(const String& path) const;
	void UpdateRenderState(float deltaTime);
};