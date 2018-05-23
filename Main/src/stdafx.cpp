// generate PCH for project
#include "stdafx.h"

// define global variables
#include "Global.hpp"
#include "GameConfig.hpp"

shared_ptr<OpenGL> g_gl;
shared_ptr<Window> g_gameWindow;
float g_aspectRatio = (16.0f / 9.0f);
Vector2i g_resolution;
shared_ptr<Application> g_application;
shared_ptr<JobSheduler> g_jobSheduler;
Input g_input;
GameConfig g_gameConfig;

// GUI
shared_ptr<GUIRenderer> g_guiRenderer;
shared_ptr<class Canvas> g_rootCanvas;
shared_ptr<class CommonGUIStyle> g_commonGUIStyle;