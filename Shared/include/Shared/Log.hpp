#pragma once
#include "Shared/String.hpp"
#include "Shared/Unique.hpp"
#include "File.hpp"
#include "FileStream.hpp"

/*
	Logging utility class
	formats loggin messages with time stamps and module names
	allows message coloring on platforms that support it
*/
class Logger : Unique
{
public:
	enum Color
	{
		Red = 0,
		Green,
		Blue,
		Yellow,
		Cyan,
		Magenta,
		White,
		Gray
	};

	enum Severity
	{
		Normal,
		Warning,
		Error,
		Info
	};

	Logger();
	~Logger();
	static Logger& Get();

	// Sets the foreground color of the output, if applicable
	void SetColor(Color color) const;

	// Log a string to the logging output, 
	void Log(const String& msg, Severity severity);

	// Write log message header, (timestamp, etc..)
	void WriteHeader(Severity severity);

	// Writes string without newline
	void Write(const String& msg);

private:
#ifdef _WIN32
	HANDLE consoleHandle;
#endif
	String moduleName;
	File m_logFile;
	FileWriter m_writer;
};

// Log to Logger::Get() with formatting string
template <typename... Args>
void Logf(const char* format, Logger::Severity severity, Args ... args)
{
	String msg = Utility::Sprintf<Args...>(format, args...);
	Logger::Get().Log(msg, severity);
}

// Log to Logger::Get()
void Log(const String& msg, Logger::Severity severity = Logger::Normal);

#ifdef _WIN32
namespace Utility
{
	String WindowsFormatMessage(uint32 code);
}
#endif
