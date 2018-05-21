#pragma once
#include "Shared/Shared.hpp"
#include "Application.hpp"
#include "Shared/Jobs.hpp"
#include "Input.hpp"
#include "GUI/GUIRenderer.hpp"
#include "GUI/Canvas.hpp"
#include "GUI/CommonGUIStyle.hpp"
#include "GameConfig.hpp"
#include "Audio/Global.hpp"
#include "Online/Global.hpp"

// variables implemented in stdafx.cpp
extern shared_ptr<OpenGL> g_gl;
extern shared_ptr<Window> g_gameWindow;
extern float g_aspectRatio;
extern Vector2i g_resolution;
extern shared_ptr<Application> g_application;
extern shared_ptr<JobSheduler> g_jobSheduler;
extern Input g_input;

extern GameConfig g_gameConfig;

// GUI
extern shared_ptr<GUIRenderer> g_guiRenderer;
extern shared_ptr<Canvas> g_rootCanvas;
extern shared_ptr<CommonGUIStyle> g_commonGUIStyle;