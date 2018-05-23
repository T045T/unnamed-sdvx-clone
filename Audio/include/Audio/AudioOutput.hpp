#pragma once

class IMixer
{
public:
	virtual void Mix(float* data, uint32& numSamples) = 0;
};

/*
	Low level audio output
*/
class AudioOutput : public Unique
{
public:
	AudioOutput();

	bool Init() const;

	// Safe to start mixing
	void Start(IMixer* mixer) const;
	// Should stop mixing
	void Stop() const;

	uint32_t GetNumChannels() const;
	uint32_t GetSampleRate() const;

	// The actual length of the buffer in seconds
	double GetBufferLength() const;

private:
	shared_ptr<class AudioOutput_Impl> m_impl;
};
