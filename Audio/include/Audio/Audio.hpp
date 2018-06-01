#pragma once
#include "AudioStream.hpp"
#include "DSP.hpp"
#include "Sample.hpp"
#include "AudioOutput.hpp"
#include <thread>
#include <mutex>
using std::thread;
using std::mutex;

/*
	Main audio manager
	keeps track of active samples and audio streams
	also handles mixing and DSP's on playing items
*/
class Audio : Unique, IMixer
{
public:
	Audio();
	virtual ~Audio();

	void start();
	void stop();

	// Registers an AudioBase to be rendered
	void Register(AudioBase* audio);
	// Removes an AudioBase so it is no longer rendered
	void Deregister(AudioBase* audio);

	void SetGlobalVolume(float vol);

	void Mix(float* data, uint32& numSamples) override;

	// Opens a stream at path
	//	settings preload loads the whole file into memory before playing
	AudioStream CreateStream(const String& path, bool preload = false);
	// Open a wav file at path
	Sample CreateSample(const String& path);

	// Target/Output sample rate
	uint32 GetSampleRate() const;

	double get_seconds_per_sample() const;
	int64 get_audio_latency() const;

	mutex lock;

private:
	unique_ptr<AudioOutput> output;

	// Used to limit rendering to a fixed number of samples (512)
	float* m_sampleBuffer = nullptr;
	uint32 m_sampleBufferLength = 384;
	uint32 m_remainingSamples = 0;

	float globalVolume = 1.0f;

	thread audioThread;

	Vector<AudioBase*> itemsToRender;
	Vector<DSP*> globalDSPs;

	LimiterDSP* limiter = nullptr;

	// Calculated audio latency by the audio driver (currently unused)
	int64 audioLatency;
};
