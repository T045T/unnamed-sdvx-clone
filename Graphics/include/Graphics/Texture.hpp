#pragma once

namespace Graphics
{
	enum class TextureWrap
	{
		Repeat,
		Mirror,
		Clamp,
	};

	enum class TextureFormat
	{
		RGBA8,
		D32,
		Invalid
	};

	class ImageRes;

	/*
		OpenGL texture wrapper, can be created from an Image object or raw data
	*/
	class TextureRes
	{
	public:
		TextureRes();
		TextureRes(const shared_ptr<ImageRes>& image);
		~TextureRes();

		void Init(Vector2i size, TextureFormat format = TextureFormat::RGBA8);
		void SetData(Vector2i size, void* pData);
		void SetMipmaps(bool enabled);
		void SetFilter(bool enabled, bool mipFiltering = true, float anisotropic = 1.0f);
		const Vector2i& GetSize() const;

		// Gives the aspect ratio correct height for a given width
		float CalculateHeight(float width) const;
		// Gives the aspect ratio correct width for a given height
		float CalculateWidth(float height) const;

		// Binds the texture to a given texture unit (default = 0)
		void Bind(uint32 index = 0) const;
		uint32 Handle() const;
		void SetWrap(TextureWrap u, TextureWrap v) const;
		TextureFormat GetFormat() const;

	private:
		uint32 m_texture = 0;
		TextureFormat m_format = TextureFormat::Invalid;
		Vector2i m_size;
		bool m_filter = true;
		bool m_mipFilter = true;
		bool m_mipmaps = false;
		float m_anisotropic = 1.0f;

		void update_filter_state() const;
	};

	typedef shared_ptr<TextureRes> Texture;
}
