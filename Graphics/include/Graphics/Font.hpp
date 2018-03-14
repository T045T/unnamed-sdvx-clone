#pragma once
#include <Graphics/ResourceTypes.hpp>

#ifdef None
#undef None
#endif

namespace Graphics
{
	/*
		A prerendered text object, contains all the vertices and texture sheets to draw itself
	*/
	class TextRes
	{
		friend class Font_Impl;
		struct FontSize* fontSize;
		std::shared_ptr<class MeshRes> mesh;
	public:
		~TextRes();
		std::shared_ptr<class TextureRes> GetTexture();
		std::shared_ptr<class MeshRes> GetMesh() { return mesh; }
		void Draw();
		Vector2 size;
	};

	/*
		Font class, can create Text objects
	*/
	class FontRes
	{
	public:
		virtual ~FontRes() = default;
		static std::shared_ptr<FontRes> Create(class OpenGL* gl, const String& assetPath);
	public:
		// Text rendering options
		enum TextOptions
		{
			None = 0,
			Monospace = 0x1,
		};

		// Renders the input string into a drawable text object
		virtual std::shared_ptr<TextRes> CreateText(const WString& str, uint32 nFontSize, TextOptions options = TextOptions::None) = 0;

	};

	typedef std::shared_ptr<FontRes> Font;
	typedef std::shared_ptr<TextRes> Text;

	DEFINE_RESOURCE_TYPE(Font, FontRes);
}
