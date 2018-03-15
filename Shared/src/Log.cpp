#include "stdafx.h"
#include "Log.hpp"
#include "File.hpp"
#include "Path.hpp"
#include "FileStream.hpp"
#include "TextStream.hpp"
#include <ctime>
#include <map>

static std::map<Logger::Color, const char*> params = {
	{Logger::Color::Red, "200;0;0"},
	{Logger::Color::Green, "0;200;0"},
	{Logger::Color::Blue, "0;70;200"},
	{Logger::Color::Yellow, "200;180;0"},
	{Logger::Color::Cyan, "0;200;200"},
	{Logger::Color::Magenta, "200;0;200"},
	{Logger::Color::Gray, "140;140;140"}
};

// Severity strings
const char* severityNames[] = {
	"Normal",
	"Warning",
	"Error",
	"Info",
};

Logger::Logger()
{
	// Store the name of the executable
	moduleName = Path::GetModuleName();

#ifdef _WIN32
	// Store console output handle
	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	// Log to file
	m_logFile.OpenWrite(Utility::Sprintf("log_%s.txt", moduleName));
	m_writer = FileWriter(m_logFile);
}

Logger::~Logger()
{
#ifndef _WIN32
	// Reset terminal colors
	printf("\x1b[39m\x1b[0m");
#endif
}

Logger& Logger::Get()
{
	static Logger logger;
	return logger;
}

void Logger::SetColor(Color color) const
{
#ifdef _WIN32
	if (consoleHandle)
	{
		static uint8 params[] =
		{
			FOREGROUND_INTENSITY | FOREGROUND_RED,
			FOREGROUND_INTENSITY | FOREGROUND_GREEN,
			FOREGROUND_INTENSITY | FOREGROUND_BLUE,
			FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN,                  // Yellow,
			FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED,                    // Cyan,
			FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,                   // Magenta,
			FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // White
			FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN,                        // Gray
		};
		SetConsoleTextAttribute(consoleHandle, params[static_cast<size_t>(color)]);
	}
#else
	if (color == Color::White)
		printf("\x1b[39m");
	else
		printf("\x1b[38;2;%sm", params[color]);
#endif
}

void Logger::Log(const String& msg, Severity severity)
{
	switch (severity)
	{
	case Normal:
		SetColor(White);
		break;
	case Info:
		SetColor(Gray);
		break;
	case Warning:
		SetColor(Yellow);
		break;
	case Error:
		SetColor(Red);
		break;
	}

	WriteHeader(severity);
	Write(msg);
	Write("\n");
}

void Logger::WriteHeader(Severity severity)
{
	// Format a timestamp string
	char timeStr[64];
	time_t currentTime = time(0);
	tm currentLocalTime;
	localtime_s(&currentLocalTime, &currentTime);
	strftime(timeStr, sizeof(timeStr), "%T", &currentLocalTime);

	// Write the formated header
	Write(Utility::Sprintf("[%s][%s] ", timeStr, severityNames[static_cast<size_t>(severity)]));
}

void Logger::Write(const String& msg)
{
#ifdef _WIN32
	OutputDebugStringA(*msg);
#endif
	printf("%s", msg.c_str());
	TextStream::Write(m_writer, msg);
}

void Log(const String& msg, Logger::Severity severity)
{
	Logger::Get().Log(msg, severity);
}

#ifdef _WIN32
String Utility::WindowsFormatMessage(uint32 code)
{
	if (code == 0)
		return "No additional info available";

	char buffer[1024] = {0};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, LANG_SYSTEM_DEFAULT, buffer, sizeof(buffer), 0);
	return buffer;
}
#endif
