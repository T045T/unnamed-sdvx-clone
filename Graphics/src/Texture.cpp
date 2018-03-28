#include "stdafx.h"
#include "OpenGL.hpp"
#include "Texture.hpp"
#include "Image.hpp"

namespace Graphics
{
	/**
	 * \throws std::runtime_error if failed to generate opengl texture
	 */
	TextureRes::TextureRes()
	{
		glGenTextures(1, &m_texture);
		if (m_texture == 0)
		{
			Log("Failed to generate texture", Logger::Severity::Error);
			throw std::runtime_error("Failed to generate texture");
		}
	}

	/**
	 * \throws std::runtime_error if failed to generate opengl texture
	 */
	TextureRes::TextureRes(const shared_ptr<ImageRes>& image)
		: TextureRes()
	{
		SetData(image->GetSize(), image->GetBits());
	}

	TextureRes::~TextureRes()
	{
		if (m_texture)
			glDeleteTextures(1, &m_texture);
	}

	void TextureRes::Init(Vector2i size, TextureFormat format)
	{
		m_format = format;
		m_size = size;

		uint32 ifmt = -1;
		uint32 fmt = -1;
		uint32 type = -1;
		if (format == TextureFormat::D32)
		{
			ifmt = GL_DEPTH_COMPONENT32;
			fmt = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		}
		else if (format == TextureFormat::RGBA8)
		{
			ifmt = GL_RGBA8;
			fmt = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			assert(false);
		}

		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, ifmt, size.x, size.y, 0, fmt, type, nullptr);
		update_filter_state();
	}

	void TextureRes::SetData(Vector2i size, void* pData)
	{
		m_format = TextureFormat::RGBA8;
		m_size = size;
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
		update_filter_state();
	}

	void TextureRes::SetMipmaps(bool enabled)
	{
		if (enabled)
		{
			if (glGenerateTextureMipmap)
				glGenerateTextureMipmap(m_texture);
			else
				glGenerateTextureMipmapEXT(m_texture, GL_TEXTURE_2D);
		}
		m_mipmaps = enabled;
		update_filter_state();
	}

	void TextureRes::SetFilter(bool enabled, bool mipFiltering, float anisotropic)
	{
		m_mipFilter = mipFiltering;
		m_filter = enabled;
		m_anisotropic = anisotropic;
		assert(m_anisotropic >= 1.0f && m_anisotropic <= 16.0f);
		update_filter_state();
	}

	const Vector2i& TextureRes::GetSize() const
	{
		return m_size;
	}

	float TextureRes::CalculateHeight(float width) const
	{
		Vector2 size = GetSize();
		float aspect = size.y / size.x;
		return aspect * width;
	}

	float TextureRes::CalculateWidth(float height) const
	{
		Vector2 size = GetSize();
		float aspect = size.x / size.y;
		return aspect * height;
	}

	void TextureRes::Bind(uint32 index) const
	{
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}

	uint32 TextureRes::Handle() const
	{
		return m_texture;
	}

	void TextureRes::SetWrap(TextureWrap u, TextureWrap v) const
	{
		uint32 wmode[] = {
			GL_REPEAT,
			GL_MIRRORED_REPEAT,
			GL_CLAMP_TO_EDGE,
		};

		if (glTextureParameteri)
		{
			glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, wmode[static_cast<size_t>(u)]);
			glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, wmode[static_cast<size_t>(v)]);
		}
		else
		{
			glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wmode[static_cast<size_t>(u)]);
			glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wmode[static_cast<size_t>(v)]);
		}
	}

	TextureFormat TextureRes::GetFormat() const
	{
		return m_format;
	}

	void TextureRes::update_filter_state() const
	{
		if (glTextureParameteri)
		{
			if (!m_mipmaps)
			{
				glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
				glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			}
			else
			{
				if (m_mipFilter)
					glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER,
						m_filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
				else
					glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER,
						m_filter ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
				glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			}
			if (GL_TEXTURE_MAX_ANISOTROPY_EXT)
			{
				glTextureParameterf(m_texture, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropic);
			}
		}
		else /* Sigh */
		{
			if (!m_mipmaps)
			{
				glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
				glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			}
			else
			{
				if (m_mipFilter)
					glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						m_filter ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
				else
					glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						m_filter ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
				glTextureParameteriEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter ? GL_LINEAR : GL_NEAREST);
			}
			if (GL_TEXTURE_MAX_ANISOTROPY_EXT)
			{
				glTextureParameterfEXT(m_texture, GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropic);
			}
		}
	}
}
