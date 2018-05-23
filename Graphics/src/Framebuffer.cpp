#include "stdafx.h"
#include "Framebuffer.hpp"
#include "Texture.hpp"
#include "OpenGL.hpp"

namespace Graphics
{
	/**
	 * \throws std::runtime_error if fails to generate framebuffer
	 */
	FramebufferRes::FramebufferRes(shared_ptr<OpenGL> gl)
		: m_gl(std::move(gl))
	{
		glGenFramebuffers(1, &m_fb);

		if (m_fb == 0)
			throw std::runtime_error("Failed to generate Framebuffer");
	}

	FramebufferRes::~FramebufferRes()
	{
		if (m_fb > 0)
			glDeleteFramebuffers(1, &m_fb);
		assert(!m_isBound);
	}

	bool FramebufferRes::AttachTexture(std::shared_ptr<TextureRes> tex)
	{
		if (!tex)
			return false;
		m_textureSize = tex->GetSize();
		uint32 texHandle = static_cast<uint32>(tex->Handle());
		TextureFormat fmt = tex->GetFormat();
		if (fmt == TextureFormat::D32)
		{
			glNamedFramebufferTexture2DEXT(m_fb, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texHandle, 0);
			m_depthAttachment = true;
		}
		else
		{
			glNamedFramebufferTexture2DEXT(m_fb, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texHandle, 0);
		}
		return IsComplete();
	}

	bool FramebufferRes::IsComplete() const
	{
		int complete = glCheckNamedFramebufferStatus(m_fb, GL_DRAW_FRAMEBUFFER);
		return complete == GL_FRAMEBUFFER_COMPLETE;
	}

	void FramebufferRes::Bind()
	{
		assert(!m_isBound);

		// Adjust viewport to frame buffer
		//glGetIntegerv(GL_VIEWPORT, &m_gl->m_lastViewport.pos.x);
		//glViewport(0, 0, m_textureSize.x, m_textureSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fb);

		if (m_depthAttachment)
		{
			GLenum drawBuffers[2] =
			{
				GL_COLOR_ATTACHMENT0,
				GL_DEPTH_ATTACHMENT
			};
			glDrawBuffers(2, drawBuffers);
		}
		else
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}

		m_isBound = true;
		m_gl->set_framebuffer(this);
	}

	void FramebufferRes::Unbind()
	{
		assert(m_isBound && m_gl->get_framebuffer() == this);

		// Restore viewport
		//Recti& vp = m_gl->m_lastViewport;
		//glViewport(vp.pos.x, vp.pos.y, vp.size.x, vp.size.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);

		m_isBound = false;
		m_gl->set_framebuffer(nullptr);
	}

	uint32 FramebufferRes::Handle() const
	{
		return m_fb;
	}
}
