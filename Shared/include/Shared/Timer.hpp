///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                           //
//  Copyright (c) 2012-2015, Jan de Graaf (jan@jlib.nl)                                                      //
//                                                                                                           //
//  Permission to use, copy, modify, and/or distribute this software for any purpose with or                 //
//  without fee is hereby granted, provided that the above copyright notice and this permission              //
//  notice appear in all copies.                                                                             //
//                                                                                                           //
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS             //
//  SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL              //
//  THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES          //
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE     //
//  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  //
//                                                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Modified version of jlib timer (http://jlib.nl/index.php?file=./code/include/jlib/timer)

#pragma once
#include <chrono>

class Timer
{
public:
	Timer()
		: m_start(std::chrono::high_resolution_clock::now())
	{}

	template <typename rep, typename period>
	Timer(std::chrono::duration<rep, period> duration)
		: m_start(std::chrono::high_resolution_clock::now())
	{
		m_start -= duration;
	}

	template <typename rep, typename period>
	Timer& operator=(std::chrono::duration<rep, period> duration)
	{
		Restart();
		m_start -= duration;
		return *this;
	}

	void Restart()
	{
		m_start = std::chrono::high_resolution_clock::now();
	}

	template <typename dur>
	inline dur Duration() const
	{
		return std::chrono::duration_cast<dur>(std::chrono::high_resolution_clock::now() - m_start);
	}

	std::chrono::nanoseconds::rep Nanoseconds() const
	{
		return Duration<std::chrono::nanoseconds>().count();
	}

	std::chrono::microseconds::rep Microseconds() const
	{
		return Duration<std::chrono::microseconds>().count();
	}

	std::chrono::milliseconds::rep Milliseconds() const
	{
		return Duration<std::chrono::milliseconds>().count();
	}

	std::chrono::seconds::rep Seconds() const
	{
		return Duration<std::chrono::seconds>().count();
	}

	std::chrono::minutes::rep Minutes() const
	{
		return Duration<std::chrono::minutes>().count();
	}

	std::chrono::hours::rep Hours() const
	{
		return Duration<std::chrono::hours>().count();
	}

	float SecondsAsFloat() const
	{
		return Duration<std::chrono::duration<float>>().count();
	}

	double SecondsAsDouble() const
	{
		return Duration<std::chrono::duration<double>>().count();
	}

	template <typename rep, typename period>
	Timer& operator+=(std::chrono::duration<rep, period> duration)
	{
		m_start -= duration;
		return *this;
	}

	template <typename rep, typename period>
	Timer& operator-=(std::chrono::duration<rep, period> duration)
	{
		m_start += duration;
		return *this;
	}

private:
	std::chrono::high_resolution_clock::time_point m_start;
};
