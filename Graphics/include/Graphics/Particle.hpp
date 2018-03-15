#pragma once
#include "Shared/Color.hpp"
#include "ParticleEmitter.hpp"

namespace Graphics
{
	// Particle instance class
	class Particle
	{
	public:
		float life = 0.0f;
		float maxLife = 0.0f;
		float rotation = 0.0f;
		float startSize = 0.0f;
		Color startColor;
		Vector3 pos;
		Vector3 velocity;
		float scale;
		float fade;
		float drag;

		bool IsAlive() const;

		void Init(ParticleEmitter* emitter);
		void Simulate(ParticleEmitter* emitter, float deltaTime);
	};
}
