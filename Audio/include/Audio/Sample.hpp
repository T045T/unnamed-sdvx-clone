#pragma once
#include "AudioBase.hpp"

/*
	Audio sample, only supports wav files in signed 16 bit stereo or mono
*/
class SampleRes : public AudioBase
{
public:
	static std::shared_ptr<SampleRes> Create(class Audio* audio, const String& path);
	virtual ~SampleRes() = default;

public:
	virtual const Buffer& GetData() const = 0;
	virtual uint32 GetBitsPerSample() const = 0;
	virtual uint32 GetNumChannels() const = 0;

	// Plays this sample from the start
	virtual void Play() = 0;
};

typedef std::shared_ptr<SampleRes> Sample;
