#pragma once
#include <Graphics/Material.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/ParticleParameter.hpp>

namespace Graphics
{
	class Particle;
	class ParticleSystemRes;

	/*
		Particle Emitter, which is a component of a particle system that handles the emission of particles together with the properties of the emitter particles
	*/
	class ParticleEmitter
	{
	public:
		~ParticleEmitter();

		// Material used for the particle
		Material material;

		// Texture to use for the particle
		Texture texture;

		// Emitter location
		Vector3 position;

		// Emitter duration
		float duration = 5.0f;

		float scale = 1.0f;

		// Amount of loops to make
		// 0 = forever
		uint32 loops = 0;

		// Particle parameter accessors
#define PARTICLE_PARAMETER(__name, __type)\
	void Set##__name(const IParticleParameter<__type>& param)\
	{\
		if(m_param_##__name)\
			delete m_param_##__name;\
		m_param_##__name = param.Duplicate();\
	}
#include <Graphics/ParticleParameters.hpp>

		// True after all loops are done playing
		bool HasFinished() const
		{
			return m_finished;
		}

		// Restarts a particle emitter
		void Reset();

		// Stop spawning any particles
		void Deactivate();

	private:
		friend ParticleSystemRes;
		friend Particle;

		// Constructed by particle system
		ParticleEmitter(ParticleSystemRes* sys);
		void Render(const RenderState& rs, float deltaTime);
		void m_ReallocatePool(uint32 newCapacity);

		float m_spawnCounter = 0;
		float m_emitterTime = 0;
		float m_emitterRate;
		bool m_deactivated = false;
		bool m_finished = false;
		uint32 m_emitterLoopIndex = 0;

		ParticleSystemRes* m_system;

		Particle* m_particles = nullptr;
		uint32 m_poolSize = 0;

		// Particle parameters private
#define PARTICLE_PARAMETER(__name, __type)\
	IParticleParameter<__type>* m_param_##__name = nullptr;
#include <Graphics/ParticleParameters.hpp>
	};
}
