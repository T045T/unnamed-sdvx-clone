#pragma once
#include "stdafx.h"
#include "Thread.hpp"

// These aren't really implementable in macOS - there's a way to set
// affinity
// (https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/index.html#//apple_ref/doc/uid/TP40006635-CH1-DontLinkElementID_2),
// but it's experimental, cannot be changed after starting a thread
// and only taken as a hint anyway.

size_t Thread::SetAffinityMask(size_t affinityMask)
{
	return 0;
}

size_t Thread::SetCurrentThreadAffinityMask(size_t affinityMask)
{
	return 0;
}
