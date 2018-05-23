#pragma once

namespace Graphics
{
	/*
		Framebuffer/Rendertarget object
		Can have textures attached to it and then render to them
	*/
	class OpenGL;

	class FramebufferRes
	{
	public:
		FramebufferRes(shared_ptr<OpenGL> gl);
		~FramebufferRes();

		bool AttachTexture(std::shared_ptr<class TextureRes> tex);
		bool IsComplete() const;
		void Bind();
		void Unbind();
		uint32 Handle() const;

	private:
		shared_ptr<OpenGL> m_gl;
		uint32 m_fb = 0;
		Vector2i m_textureSize;
		bool m_isBound = false;
		bool m_depthAttachment = false;
	};

	typedef shared_ptr<FramebufferRes> Framebuffer;
}