#pragma once
#include <Graphics/ResourceTypes.hpp>
#include <Graphics/ParticleEmitter.hpp>

namespace Graphics
{
	/*
		Particles system
		contains emitters and handles the cleanup/lifetime of them.
	*/
	class ParticleSystemRes
	{
	public:
		static std::shared_ptr<ParticleSystemRes> Create(class OpenGL* gl);

		// Create a new emitter
		std::shared_ptr<ParticleEmitter> add_emitter();
		void render(const class RenderState& rs, float deltaTime);
		// Removes all active particle systems
		void reset();

		OpenGL* gl;

	private:
		friend ParticleEmitter;

		Vector<std::shared_ptr<ParticleEmitter>> m_emitters;
	};

	typedef std::shared_ptr<ParticleSystemRes> ParticleSystem;

	DEFINE_RESOURCE_TYPE(ParticleSystem, ParticleSystemRes);
}
