#pragma once
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

		// Create a new emitter
		shared_ptr<ParticleEmitter> add_emitter();

		void render(const class RenderState& rs, float deltaTime);

		// Removes all active particle systems
		void reset();

	private:
		friend ParticleEmitter;

		shared_ptr<OpenGL> m_gl;
		Vector<shared_ptr<ParticleEmitter>> m_emitters;
	};

	typedef shared_ptr<ParticleSystemRes> ParticleSystem;
}
