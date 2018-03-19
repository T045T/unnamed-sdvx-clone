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
		ParticleSystemRes(shared_ptr<OpenGL> gl);

		static shared_ptr<ParticleSystemRes> Create(shared_ptr<OpenGL> gl);
		// Create a new emitter
		std::shared_ptr<ParticleEmitter> add_emitter();
		void render(const class RenderState& rs, float deltaTime);
		// Removes all active particle systems
		void reset();

	private:
		friend ParticleEmitter;

		shared_ptr<OpenGL> m_gl;
		Vector<std::shared_ptr<ParticleEmitter>> m_emitters;
	};

	typedef std::shared_ptr<ParticleSystemRes> ParticleSystem;

	DEFINE_RESOURCE_TYPE(ParticleSystem, ParticleSystemRes);
}
