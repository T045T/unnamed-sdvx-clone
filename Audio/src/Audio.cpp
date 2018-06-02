#include "stdafx.h"
#include "Audio.hpp"
#include "AudioStream.hpp"
#include "AudioOutput.hpp"
#include "DSP.hpp"

shared_ptr<Audio> g_audio;

void Audio::Mix(float* data, uint32& numSamples)
{
#if _DEBUG
	static const uint32 guardBand = 1024;
#else
	static const uint32 guardBand = 0;
#endif

	// Per-Channel data buffer
	auto tempData = new float[m_sampleBufferLength * 2 + guardBand];

	uint32 outputChannels = this->output->GetNumChannels();
	memset(data, 0, numSamples * sizeof(float) * outputChannels);

	uint32 currentNumberOfSamples = 0;
	while (currentNumberOfSamples < numSamples)
	{
		// Generate new sample
		if (m_remainingSamples <= 0)
		{
			// Clear sample buffer storing a fixed amount of samples
			memset(m_sampleBuffer, 0, sizeof(float) * 2 * m_sampleBufferLength);

			// Render items
			{
				std::scoped_lock l(lock);
				for (auto& item : itemsToRender)
				{
					// Clearn per-channel data (and guard buffer in debug mode)
					memset(tempData, 0, sizeof(float) * (2 * m_sampleBufferLength + guardBand));
					item->Process(tempData, m_sampleBufferLength);
#if _DEBUG
					// Check for memory corruption
					for (uint32 i = 0; i < guardBand; i++)
					{
						assert(guardBuffer[i] == 0);
					}
#endif
					item->ProcessDSPs(tempData, m_sampleBufferLength);
#if _DEBUG
					// Check for memory corruption
					for (uint32 i = 0; i < guardBand; i++)
					{
						assert(guardBuffer[i] == 0);
					}
#endif

					// Mix into buffer and apply volume scaling
					for (uint32 i = 0; i < m_sampleBufferLength; i++)
					{
						m_sampleBuffer[i * 2 + 0] += tempData[i * 2] * item->get_volume();
						m_sampleBuffer[i * 2 + 1] += tempData[i * 2 + 1] * item->get_volume();
					}
				}

				// Process global DSPs
				for (auto dsp : globalDSPs)
				{
					dsp->Process(m_sampleBuffer, m_sampleBufferLength);
				}
			}

			// Apply volume levels
			for (uint32 i = 0; i < m_sampleBufferLength; i++)
			{
				m_sampleBuffer[i * 2 + 0] *= globalVolume;
				m_sampleBuffer[i * 2 + 1] *= globalVolume;
			}

			// Set new remaining buffer data
			m_remainingSamples = m_sampleBufferLength;
		}

		// Copy samples from sample buffer
		const uint32 sampleOffset = m_sampleBufferLength - m_remainingSamples;
		const uint32 maxSamples = Math::Min(numSamples - currentNumberOfSamples, m_remainingSamples);
		for (uint32 c = 0; c < outputChannels; c++)
		{
			if (c < 2)
			{
				for (uint32 i = 0; i < maxSamples; i++)
				{
					data[(currentNumberOfSamples + i) * outputChannels + c] = m_sampleBuffer[(sampleOffset + i) * 2 + c];
				}
			}
			// TODO: Mix to surround channels as well?
		}
		m_remainingSamples -= maxSamples;
		currentNumberOfSamples += maxSamples;
	}

	delete[] tempData;
}

void Audio::start()
{
	std::scoped_lock l(lock);
	if (m_sampleBuffer || limiter) {
		Log("Audio.start() called twice!", Logger::Error);
		return;
	}
	m_sampleBuffer = new float[2 * m_sampleBufferLength];

	limiter = new LimiterDSP();
	limiter->audio = this;
	limiter->releaseTime = 0.2f;
	globalDSPs.Add(limiter);
	output->Start(this);
}

void Audio::stop()
{
	std::scoped_lock l(lock);
	output->Stop();
	delete limiter;
	globalDSPs.Remove(limiter);
	limiter = nullptr;

	delete[] m_sampleBuffer;
	m_sampleBuffer = nullptr;
}

void Audio::Register(AudioBase* audio)
{
	std::scoped_lock l(lock);
	itemsToRender.AddUnique(audio);
	audio->audio = this;
}

void Audio::Deregister(AudioBase* audio)
{
	std::scoped_lock l(lock);
	itemsToRender.Remove(audio);
	audio->audio = nullptr;
}

/**
 * \throws std::runtime_error Failed to init AudioOutput
 */
Audio::Audio()
{
	// Enforce single instance
	assert(g_audio == nullptr);

	audioLatency = 0;

	output = make_unique<AudioOutput>();
	if (!output->Init())
		throw runtime_error("Failed to init AudioOutput");

	start();
}

Audio::~Audio()
{
	stop();
}

void Audio::SetGlobalVolume(float vol)
{
	globalVolume = vol;
}

uint32 Audio::GetSampleRate() const
{
	return output->GetSampleRate();
}

double Audio::get_seconds_per_sample() const
{
	return 1.0 / static_cast<double>(GetSampleRate());
}

int64 Audio::get_audio_latency() const
{
	return audioLatency;
}

AudioStream Audio::CreateStream(const String& path, bool preload)
{
	return AudioStreamRes::Create(this, path, preload);
}

Sample Audio::CreateSample(const String& path)
{
	return SampleRes::Create(this, path);
}
