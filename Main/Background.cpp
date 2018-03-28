#include "stdafx.h"
#include "Background.hpp"
#include "Application.hpp"
#include "GameConfig.hpp"
#include "Background.hpp"
#include "Game.hpp"
#include "Track.hpp"
#include "Camera.hpp"
#include "Global.hpp"

/* Background template for fullscreen effects */
class FullscreenBackground : public Background
{
public:
	virtual bool Init(bool foreground) override
	{
		fullscreenMesh = MeshGenerators::Quad(g_gl, Vector2(-1.0f), Vector2(2.0f));
		return true;
	}

	void UpdateRenderState(float deltaTime)
	{
		renderState = g_application->GetRenderStateBase();
	}

	virtual void Render(float deltaTime) override
	{
		assert(fullscreenMaterial);

		// Render a fullscreen quad
		RenderQueue rq(g_gl, renderState);
		rq.Draw(Transform(), fullscreenMesh, fullscreenMaterial, fullscreenMaterialParams);
		rq.Process();
	}

	RenderState renderState;
	Mesh fullscreenMesh;
	Material fullscreenMaterial;
	Texture backgroundTexture;
	MaterialParameterSet fullscreenMaterialParams;
	float clearTransition = 0.0f;
};

class TestBackground : public FullscreenBackground
{
	virtual bool Init(bool foreground) override
	{
		if (!FullscreenBackground::Init(foreground))
			return false;

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

		return true;
	}

	virtual void Render(float deltaTime) override
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

		FullscreenBackground::Render(deltaTime);
	}


	/**
	 * \throws std::runtime_error if failed to create shader/material/texture
	 */
	Material LoadBackgroundMaterial(const String& path)
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
	Texture LoadBackgroundTexture(const String& path)
	{
		return make_shared<TextureRes>(make_shared<ImageRes>(path));
	}
};


Background* CreateBackground(class Game* game, bool foreground /* = false*/)
{
	Background* bg = new TestBackground();
	bg->game = game;
	if (!bg->Init(foreground))
	{
		delete bg;
		return nullptr;
	}
	return bg;
}
