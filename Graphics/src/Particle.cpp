#include "stdafx.h"
#include "Particle.hpp"

namespace Graphics
{
	bool Particle::IsAlive() const
	{
		return life > 0.0f;
	}

	void Particle::Init(ParticleEmitter* emitter)
	{
		const float& et = emitter->m_emitterRate;
		life = maxLife = emitter->m_param_Lifetime->Init(et);
		pos = emitter->m_param_StartPosition->Init(et) * emitter->scale;

		// Velocity of startvelocity and spawn offset scale
		velocity = emitter->m_param_StartVelocity->Init(et) * emitter->scale;
		float spawnVelScale = emitter->m_param_SpawnVelocityScale->Init(et);
		if (spawnVelScale > 0)
			velocity += pos.Normalized() * spawnVelScale * emitter->scale;

		// Add emitter offset to location
		pos += emitter->position;

		startColor = emitter->m_param_StartColor->Init(et);
		rotation = emitter->m_param_StartRotation->Init(et);
		startSize = emitter->m_param_StartSize->Init(et) * emitter->scale;
		drag = emitter->m_param_StartDrag->Init(et);
	}

	void Particle::Simulate(ParticleEmitter* emitter, float deltaTime)
	{
		float c = 1 - life / maxLife;

		// Add gravity
		velocity += emitter->m_param_Gravity->Sample(emitter->m_emitterTime) * deltaTime * emitter->scale;
		pos += velocity * deltaTime;

		// Add drag
		velocity += -velocity * deltaTime * drag;

		fade = emitter->m_param_FadeOverTime->Sample(c);
		scale = emitter->m_param_ScaleOverTime->Sample(c);
		life -= deltaTime;
	}
}
