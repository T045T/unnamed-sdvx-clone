#include "stdafx.h"
#include "Background.hpp"
#include "Application.hpp"
#include "GameConfig.hpp"
#include "Camera.hpp"
#include "Game.hpp"
#include "Track.hpp"
#include "Global.hpp"

Background::Background(class Game* game_, bool foreground)
	: game(game_)
{
	fullscreenMesh = MeshGenerators::Quad(g_gl, Vector2(-1.0f), Vector2(2.0f));

	/// TODO: Handle invalid background configurations properly
	/// e.g. missing bg_texture.png and such
	String skin = g_gameConfig.GetString(GameConfigKeys::Skin);
	String matPath = "";
	if (foreground)
	{
		matPath = game->GetBeatmap()->GetMapSettings().foregroundPath;
		String texPath = "textures/fg_texture.png";
		if (matPath.length() > 3 && matPath.substr(matPath.length() - 3, 3) == ".fs")
		{
			matPath = Path::Normalize(game->GetMapRootPath() + Path::sep + matPath);
			texPath = Path::Normalize(game->GetMapRootPath() + Path::sep + "fg_texture.png");
			CheckedLoad(backgroundTexture = g_application->LoadTexture(texPath, true));
		}
		else
		{
			matPath = "skins/" + skin + "/shaders/foreground.fs";
			CheckedLoad(backgroundTexture = g_application->LoadTexture("fg_texture.png"));
		}
	}
	else
	{
		matPath = game->GetBeatmap()->GetMapSettings().backgroundPath;
		String texPath = "textures/bg_texture.png";
		if (matPath.length() > 3 && matPath.substr(matPath.length() - 3, 3) == ".fs")
		{
			matPath = Path::Normalize(game->GetMapRootPath() + Path::sep + matPath);
			texPath = Path::Normalize(game->GetMapRootPath() + Path::sep + "bg_texture.png");
			CheckedLoad(backgroundTexture = g_application->LoadTexture(texPath, true));
		}
		else
		{
			matPath = "skins/" + skin + "/shaders/background.fs";
			CheckedLoad(backgroundTexture = g_application->LoadTexture("bg_texture.png"));
		}
	}

	CheckedLoad(fullscreenMaterial = LoadBackgroundMaterial(matPath));
	fullscreenMaterial->opaque = !foreground;
}

void Background::Render(float deltaTime)
{
	UpdateRenderState(deltaTime);

	Vector3 timing;
	const TimingPoint& tp = game->GetPlayback().GetCurrentTimingPoint();
	timing.x = game->GetPlayback().GetBeatTime();
	timing.z = game->GetPlayback().GetLastTime() / 1000.0f;
	// every 1/4 tick
	float tickTime = fmodf(timing.x * (float)tp.numerator, 1.0f);
	//timing.y = powf(tickTime, 2);
	timing.y = powf(1.0f - tickTime, 1);
	//if(tickTime > 0.7f)
	//	timing.y += ((tickTime - 0.7f) / 0.3f) * 0.8f; // Gradual build up again

	bool cleared = game->GetScoring().currentGauge >= 0.70f;

	if (cleared)
		clearTransition += deltaTime / tp.beatDuration * 1000;
	else
		clearTransition -= deltaTime / tp.beatDuration * 1000;

	clearTransition = Math::Clamp(clearTransition, 0.0f, 1.0f);

	Vector3 trackEndWorld = Vector3(0.0f, 25.0f, 0.0f);
	Vector2i screenCenter = Vector2i(g_resolution.x / 2, game->GetCamera().GetHorizonHeight());
	Vector2i shakeOffset = game->GetCamera().GetShakeOffset().xy() * g_resolution * 0.15f;
	screenCenter += shakeOffset;

	float tilt = game->GetCamera().GetRoll();
	fullscreenMaterialParams.SetParameter("clearTransition", clearTransition);
	fullscreenMaterialParams.SetParameter("tilt", tilt);
	fullscreenMaterialParams.SetParameter("mainTex", backgroundTexture);
	fullscreenMaterialParams.SetParameter("screenCenter", screenCenter);
	fullscreenMaterialParams.SetParameter("timing", timing);

	assert(fullscreenMaterial);

	// Render a fullscreen quad
	RenderQueue rq(g_gl, renderState);
	rq.Draw(Transform(), fullscreenMesh, fullscreenMaterial, fullscreenMaterialParams);
	rq.Process();
}

/**
 * \throws std::runtime_error if failed to create shader/material/texture
 */
Material Background::LoadBackgroundMaterial(const String& path) const
{
	String skin = g_gameConfig.GetString(GameConfigKeys::Skin);
	String pathV = String("skins/" + skin + "/shaders/") + "background" + ".vs";
	String pathF = path;
	String pathG = String("skins/" + skin + "/shaders/") + "background" + ".gs";
	Material ret = make_shared<MaterialRes>(g_gl, pathV, pathF);
	// Additionally load geometry shader
	if (Path::FileExists(pathG))
	{
		Shader gshader = make_shared<ShaderRes>(g_gl, ShaderType::Geometry, pathG);
		assert(gshader);
		ret->AssignShader(ShaderType::Geometry, gshader);
	}
	assert(ret);
	return ret;
}

/**
 * \throws std::runtime_error if failed to create Texture/Image
 */
Texture Background::LoadBackgroundTexture(const String& path) const
{
	return make_shared<TextureRes>(make_shared<ImageRes>(path));
}

void Background::UpdateRenderState(float deltaTime)
{
	renderState = g_application->GetRenderStateBase();
}
