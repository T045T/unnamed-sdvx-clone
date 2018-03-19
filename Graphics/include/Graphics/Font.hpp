#pragma once
#include <Graphics/ResourceTypes.hpp>
#include "stdafx.h"
#include "Font.hpp"
#include "ResourceManagers.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include "ftcache.h"

#ifdef None
#undef None
#endif

namespace Graphics
{
	class FontSize;

	// A prerendered text object, contains all the vertices and texture sheets to draw itself
	class TextRes
	{
	public:
		std::shared_ptr<TextureRes> get_texture() const;
		std::shared_ptr<MeshRes> get_mesh() const;
		void draw() const;
		Vector2 size;

	private:
		friend class FontRes;
		FontSize* fontSize;
		std::shared_ptr<MeshRes> mesh;
	};

	// Font class, can create Text objects
	class FontRes
	{
	public:
		enum TextOptions
		{
			None = 0,
			Monospace = 0x1,
		};

		FontRes(shared_ptr<OpenGL> gl, const String& assetPath);
		~FontRes();

		FontSize* get_size(uint32 nSize);

		static shared_ptr<FontRes> create(shared_ptr<OpenGL> gl, const String& assetPath);

		// Renders the input string into a drawable text object
		shared_ptr<TextRes> create_text(const WString& str, uint32 nFontSize, TextOptions options = TextOptions::None);

	private:
		FT_Face m_face;
		Buffer m_data;

		Map<uint32, FontSize*> m_sizes;
		uint32 m_currentSize = 0;

		shared_ptr<OpenGL> m_gl;
	};

	typedef std::shared_ptr<FontRes> Font;
	typedef std::shared_ptr<TextRes> Text;

	struct CachedText
	{
		Text text;
		float lastUsage;
	};

	class FontLibrary
	{
	public:
		FontLibrary();
		~FontLibrary();

		bool LoadFallbackFont();
		Buffer loadedFallbackFont;
	};

	struct CharInfo
	{
		uint32 glyphID;
		float advance;
		int32 leftOffset;
		int32 topOffset;
		Recti coords;
	};

	// Prevents continuous recreation of text that doesn't change
	class TextCache : public Map<WString, CachedText>
	{
	public:
		void update();
		Text get_text(const WString& key);
		void add_text(const WString& key, Text obj);

	private:
		Timer timer;
	};

	class FontSize
	{
	public:
		FontSize(shared_ptr<OpenGL> gl, FT_Face& face);

		SpriteMap spriteMap;
		Texture textureMap;
		FT_Face face;
		Vector<CharInfo> infos;
		Map<wchar_t, uint32> infoByChar;
		bool bUpdated = false;
		float lineHeight;
		TextCache cache;

		const CharInfo& get_char_info(wchar_t t);
		Texture get_texture_map();

	private:
		const CharInfo& add_char_info(wchar_t t);

		shared_ptr<OpenGL> m_gl;
	};

	DEFINE_RESOURCE_TYPE(Font, FontRes);
}
