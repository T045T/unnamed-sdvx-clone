#pragma once
#include <Graphics/ResourceTypes.hpp>

namespace Graphics
{
	/*
		Framebuffer/Rendertarget object
		Can have textures attached to it and then render to them
	*/
	class FramebufferRes
	{
	public:
		virtual ~FramebufferRes() = default;
		static std::shared_ptr<FramebufferRes> Create(class OpenGL* gl);
	public:
		virtual bool AttachTexture(std::shared_ptr<class TextureRes> tex) = 0;
		virtual bool IsComplete() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual uint32 Handle() const = 0;
	};

	typedef std::shared_ptr<FramebufferRes> Framebuffer;

	DEFINE_RESOURCE_TYPE(Framebuffer, FramebufferRes);
}
