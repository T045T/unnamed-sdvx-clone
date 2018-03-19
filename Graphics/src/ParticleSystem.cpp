#include "stdafx.h"
#include "OpenGL.hpp"
#include "Particle.hpp"
#include "ParticleSystem.hpp"
#include "Mesh.hpp"
#include "VertexFormat.hpp"
#include <Graphics/ResourceManagers.hpp>

namespace Graphics
{
	struct ParticleVertex : VertexFormat<Vector3, Vector4, Vector4>
	{
		ParticleVertex(Vector3 pos, Color color, Vector4 params)
			: pos(pos), color(color), params(params)
		{};
		Vector3 pos;
		Color color;
		// X = scale
		// Y = rotation
		// Z = animation frame
		Vector4 params;
	};

	ParticleSystemRes::ParticleSystemRes(shared_ptr<OpenGL> gl)
		: m_gl(std::move(gl))
	{}

	shared_ptr<ParticleSystemRes> ParticleSystemRes::Create(shared_ptr<OpenGL> gl)
	{
		const auto impl = make_shared<ParticleSystemRes>(gl);
		return GetResourceManager<ResourceType::ParticleSystem>().Register(impl);
	}

	std::shared_ptr<ParticleEmitter> ParticleSystemRes::add_emitter()
	{
		auto newEmitter = std::shared_ptr<ParticleEmitter>(new ParticleEmitter(this));
		m_emitters.Add(newEmitter);
		return newEmitter;
	}

	void ParticleSystemRes::render(const RenderState& rs, float deltaTime)
	{
		// Enable blending for all particles
		glEnable(GL_BLEND);

		// Tick all emitters and remove old ones
		for (auto it = m_emitters.begin(); it != m_emitters.end();)
		{
			(*it)->Render(rs, deltaTime);

			if (it->use_count() == 1)
			{
				if ((*it)->HasFinished())
				{
					// Remove unreferenced and finished emitters
					it = m_emitters.erase(it);
					continue;
				}
				else if ((*it)->loops == 0)
				{
					// Deactivate unreferenced infinte duration emitters
					(*it)->Deactivate();
				}
			}

			++it;
		}
	}

	void ParticleSystemRes::reset()
	{
		for (auto em : m_emitters)
			em.reset();

		m_emitters.clear();
	}

	ParticleEmitter::ParticleEmitter(ParticleSystemRes* sys)
		: m_system(sys)
	{
		// Set parameter defaults
#define PARTICLE_DEFAULT(__name, __value)\
	Set##__name(__value);
#include "ParticleParameters.hpp"
	}

	ParticleEmitter::~ParticleEmitter()
	{
		// Cleanup particle parameters
#define PARTICLE_PARAMETER(__name, __type)\
	if(m_param_##__name){\
		delete m_param_##__name; m_param_##__name = nullptr; }
#include "ParticleParameters.hpp"

		delete[] m_particles;
	}

	void ParticleEmitter::m_ReallocatePool(uint32 newCapacity)
	{
		Particle* oldParticles = m_particles;
		const uint32 oldSize = m_poolSize;

		// Create new pool
		m_poolSize = newCapacity;
		if (newCapacity > 0)
		{
			m_particles = new Particle[m_poolSize];
			memset(m_particles, 0, m_poolSize * sizeof(Particle));
		}
		else
		{
			m_particles = nullptr;
		}

		if (oldParticles && m_particles)
		{
			memcpy(m_particles, oldParticles, Math::Min(oldSize, m_poolSize) * sizeof(Particle));
		}

		delete[] oldParticles;
	}

	void ParticleEmitter::Render(const class RenderState& rs, float deltaTime)
	{
		if (m_finished)
			return;

		const uint32 maxDuration = static_cast<uint32>(ceilf(m_param_Lifetime->GetMax()));
		const uint32 maxSpawns = static_cast<uint32>(ceilf(m_param_SpawnRate->GetMax()));
		uint32 maxParticles = maxSpawns * maxDuration;
		// Round up to 64
		maxParticles = static_cast<uint32>(ceil(static_cast<float>(maxParticles) / 64.0f)) * 64;

		if (maxParticles > m_poolSize)
			m_ReallocatePool(maxParticles);

		// Resulting vertex bufffer
		Vector<ParticleVertex> verts;

		// Increment emitter time
		m_emitterTime += deltaTime;
		while (m_emitterTime > duration)
		{
			m_emitterTime -= duration;
			m_emitterLoopIndex++;
		}
		m_emitterRate = m_emitterTime / duration;

		// Increment spawn counter
		m_spawnCounter += deltaTime * m_param_SpawnRate->Sample(m_emitterRate);

		uint32 numSpawns = 0;
		float spawnTimeOffset = 0.0f;
		float spawnTimeOffsetStep = 0;
		if (loops > 0 && m_emitterLoopIndex >= loops) // Should spawn particles ?
			m_deactivated = true;

		if (!m_deactivated)
		{
			// Calculate number of new particles to spawn
			float spawnsf;
			m_spawnCounter = modff(m_spawnCounter, &spawnsf);
			numSpawns = (uint32)spawnsf;
			spawnTimeOffsetStep = deltaTime / spawnsf;
		}

		bool updatedSomething = false;
		for (uint32 i = 0; i < m_poolSize; i++)
		{
			Particle& p = m_particles[i];

			bool render = false;
			if (!m_particles[i].IsAlive())
			{
				// Try to spawn a new particle in this slot
				if (numSpawns > 0)
				{
					p.Init(this);
					p.Simulate(this, spawnTimeOffset);
					spawnTimeOffset += spawnTimeOffsetStep;
					numSpawns--;
					render = true;
				}
			}
			else
			{
				p.Simulate(this, deltaTime);
				render = true;
				updatedSomething = true;
			}

			if (render)
				verts.Add({p.pos, p.startColor.WithAlpha(p.fade), Vector4(p.startSize * p.scale, p.rotation, 0, 0)});
		}

		if (m_deactivated)
			m_finished = !updatedSomething;

		MaterialParameterSet params;
		if (texture)
			params.SetParameter("mainTex", texture);

		material->Bind(rs, params);

		// Select blending mode based on material
		switch (material->blendMode)
		{
		case MaterialBlendMode::Normal:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case MaterialBlendMode::Additive:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case MaterialBlendMode::Multiply:
			glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
			break;
		}

		// Create vertex buffer
		Mesh mesh = MeshRes::Create();

		mesh->SetData(verts);
		mesh->SetPrimitiveType(PrimitiveType::PointList);

		mesh->Draw();
		mesh.reset();
	}

	void ParticleEmitter::Reset()
	{
		m_deactivated = false;
		m_finished = false;
		delete[] m_particles;
		m_particles = nullptr;
		m_emitterLoopIndex = 0;
		m_emitterTime = 0;
		m_spawnCounter = 0;
		m_poolSize = 0;
	}

	void ParticleEmitter::Deactivate()
	{
		m_deactivated = true;
	}
}
