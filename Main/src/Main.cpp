#include "stdafx.h"
#include "Application.hpp"
#include "Global.hpp"

#ifdef _WIN32
// Windows entry point
int32 __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_application = make_shared<Application>();
	g_application->SetCommandLine(GetCommandLineA());
	return g_application->Run();
}
#else
// Linux entry point
int main(int argc, char** argv)
{
	g_application = make_shared<Application>();
	g_application->SetCommandLine(argc, argv);
	return g_application->Run();
}
#endif
