#include "stdafx.h"
#include "Font.hpp"
#include "ResourceManagers.hpp"
#include "Image.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include <Shared/Timer.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Graphics
{
	FT_Library library;
	FT_Face fallbackFont;
	uint32 fallbackFontSize = 0;

	std::shared_ptr<class TextureRes> TextRes::get_texture() const
	{
		return fontSize->get_texture_map();
	}

	std::shared_ptr<class MeshRes> TextRes::get_mesh() const
	{
		return mesh;
	}

	void TextRes::draw() const
	{
		get_texture()->Bind();
		mesh->Draw();
	}

	FontRes::FontRes(OpenGL* gl, const String& assetPath)
		: m_gl(gl)
	{
		File in;
		if (!in.OpenRead(assetPath))
			throw std::runtime_error("Failed to open file");

		m_data.resize(in.GetSize());
		if (m_data.empty())
			throw std::runtime_error("data empty!");

		in.Read(&m_data.front(), m_data.size());

		if (FT_New_Memory_Face(library, m_data.data(), static_cast<FT_Long>(m_data.size()), 0, &m_face) != 0)
			throw std::runtime_error("Failed to create new memory face");

		if (FT_Select_Charmap(m_face, FT_ENCODING_UNICODE) != 0)
			throw std::runtime_error("Failed to select charmap");
	}

	FontRes::~FontRes()
	{
		m_sizes.clear();
		FT_Done_Face(m_face);
	}

	FontSize* FontRes::get_size(uint32 nSize)
	{
		if (m_currentSize != nSize)
		{
			FT_Set_Pixel_Sizes(m_face, 0, nSize);
			m_currentSize = nSize;
		}
		if (fallbackFontSize != nSize)
		{
			FT_Set_Pixel_Sizes(fallbackFont, 0, nSize);
			fallbackFontSize = nSize;
		}

		auto it = m_sizes.find(nSize);
		if (it != m_sizes.end())
			return it->second;

		const auto pMap = new FontSize(m_gl, m_face);
		m_sizes.Add(nSize, pMap);
		return pMap;
	}

	Font FontRes::Create(OpenGL* gl, const String& assetPath)
	{
		try
		{
			const auto pImpl = new FontRes(gl, assetPath);
			return GetResourceManager<ResourceType::Font>().Register(pImpl);
		}
		catch (std::runtime_error& e)
		{
			Logf("%s", Logger::Severity::Warning, e.what());
			return Font();
		}
	}

	std::shared_ptr<TextRes> FontRes::create_text(const WString& str, uint32 nFontSize, TextOptions options)
	{
		FontSize* size = get_size(nFontSize);

		Text cachedText = size->cache.get_text(str);
		if (cachedText)
			return cachedText;

		struct TextVertex : public VertexFormat<Vector2, Vector2>
		{
			TextVertex(Vector2 point, Vector2 uv) : pos(point), tex(uv) {}
			Vector2 pos;
			Vector2 tex;
		};

		auto ret = new TextRes();
		ret->mesh = MeshRes::Create(m_gl);

		const float monospaceWidth = size->get_char_info(L'_').advance;

		Vector<TextVertex> vertices;
		Vector2 pen;
		for (wchar_t c : str)
		{
			const CharInfo& info = size->get_char_info(c);

			if (info.coords.size.x != 0 && info.coords.size.y != 0)
			{
				Vector2 corners[4];
				corners[0] = Vector2(0, 0);
				corners[1] = Vector2(static_cast<float>(info.coords.size.x), 0);
				corners[2] = Vector2(static_cast<float>(info.coords.size.x), static_cast<float>(info.coords.size.y));
				corners[3] = Vector2(0, static_cast<float>(info.coords.size.y));

				Vector2 offset = Vector2(pen.x, pen.y);
				offset.x += info.leftOffset;
				offset.y += nFontSize - info.topOffset;
				if ((options & TextOptions::Monospace) != 0)
				{
					offset.x += (monospaceWidth - info.coords.size.x) * 0.5f;
				}
				pen.x = floorf(pen.x);
				pen.y = floorf(pen.y);

				vertices.emplace_back(offset + corners[2],
					corners[2] + info.coords.pos);
				vertices.emplace_back(offset + corners[0],
					corners[0] + info.coords.pos);
				vertices.emplace_back(offset + corners[1],
					corners[1] + info.coords.pos);

				vertices.emplace_back(offset + corners[3],
					corners[3] + info.coords.pos);
				vertices.emplace_back(offset + corners[0],
					corners[0] + info.coords.pos);
				vertices.emplace_back(offset + corners[2],
					corners[2] + info.coords.pos);
			}

			if (c == L'\n')
			{
				pen.x = 0.0f;
				pen.y += size->lineHeight;
				ret->size.y = pen.y;
			}
			else if (c == L'\t')
			{
				const CharInfo& space = size->get_char_info(L' ');
				pen.x += space.advance * 3.0f;
			}
			else
			{
				if ((options & TextOptions::Monospace) != 0)
					pen.x += monospaceWidth;
				else
					pen.x += info.advance;
			}
			ret->size.x = std::max(ret->size.x, pen.x);
		}

		ret->size.y += size->lineHeight;

		ret->fontSize = size;
		ret->mesh->SetData(vertices);
		ret->mesh->SetPrimitiveType(PrimitiveType::TriangleList);

		Text textObj = std::shared_ptr<TextRes>(ret);
		// Insert into cache
		size->cache.add_text(str, textObj);
		return textObj;
	}

	bool FontLibrary::LoadFallbackFont()
	{
		bool success = true;

		// Load fallback font
		File file;
		if (!file.OpenRead("fonts/fallbackfont.otf"))
			return false;

		loadedFallbackFont.resize(file.GetSize());
		file.Read(loadedFallbackFont.data(), loadedFallbackFont.size());
		file.Close();

		success = success && FT_New_Memory_Face(library, loadedFallbackFont.data(), static_cast<uint32>(loadedFallbackFont.size()), 0, &fallbackFont) == 0;
		success = success && FT_Select_Charmap(fallbackFont, FT_ENCODING_UNICODE) == 0;
		return success;
	}

	void TextCache::update()
	{
		const float currentTime = timer.SecondsAsFloat();
		for (auto it = begin(); it != end();)
		{
			const float durationSinceUsed = currentTime - it->second.lastUsage;
			if (durationSinceUsed > 1.0f)
			{
				it = erase(it);
				continue;
			}
			++it;
		}
	}

	Text TextCache::get_text(const WString& key)
	{
		const auto it = find(key);
		if (it != end())
		{
			it->second.lastUsage = timer.SecondsAsFloat();
			return it->second.text;
		}
		return Text();
	}

	void TextCache::add_text(const WString& key, Text obj)
	{
		update();
		Add(key, {obj, timer.SecondsAsFloat()});
	}

	FontSize::FontSize(OpenGL* gl, FT_Face& face)
		: face(face), m_gl(gl)
	{
		spriteMap = SpriteMapRes::Create();
		textureMap = TextureRes::Create(m_gl);
		lineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;
	}

	const CharInfo& FontSize::get_char_info(wchar_t t)
	{
		const auto it = infoByChar.find(t);
		if (it == infoByChar.end())
			return add_char_info(t);
		return infos[it->second];
	}

	Texture FontSize::get_texture_map()
	{
		if (bUpdated)
		{
			textureMap = spriteMap->GenerateTexture(m_gl);
			bUpdated = false;
		}
		return textureMap;
	}

	const CharInfo& FontSize::add_char_info(wchar_t t)
	{
		bUpdated = true;
		infoByChar.Add(t, static_cast<uint32>(infos.size()));
		infos.emplace_back();
		CharInfo& ci = infos.back();

		FT_Face* pFace = &face;

		ci.glyphID = FT_Get_Char_Index(*pFace, t);
		if (ci.glyphID == 0)
		{
			pFace = &fallbackFont;
			ci.glyphID = FT_Get_Char_Index(*pFace, t);
		}
		FT_Load_Glyph(*pFace, ci.glyphID, FT_LOAD_DEFAULT);

		if ((*pFace)->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			FT_Render_Glyph((*pFace)->glyph, FT_RENDER_MODE_NORMAL);
		}

		ci.topOffset = (*pFace)->glyph->bitmap_top;
		ci.leftOffset = (*pFace)->glyph->bitmap_left;
		ci.advance = static_cast<float>((*pFace)->glyph->advance.x) / 64.0f;

		Image img = ImageRes::Create(Vector2i((*pFace)->glyph->bitmap.width, (*pFace)->glyph->bitmap.rows));
		Colori* pDst = img->GetBits();
		uint8* pSrc = (*pFace)->glyph->bitmap.buffer;
		uint32 nLen = (*pFace)->glyph->bitmap.width * (*pFace)->glyph->bitmap.rows;
		for (uint32 i = 0; i < nLen; i++)
		{
			pDst[0].w = pSrc[0];
			reinterpret_cast<VectorMath::VectorBase<uint8, 3>&>(pDst[0]) = VectorMath::VectorBase<uint8, 3>(255, 255, 255);
			pSrc++;
			pDst++;
		}
		const uint32 nIndex = spriteMap->AddSegment(img);
		ci.coords = spriteMap->GetCoords(nIndex);

		return ci;
	}

	FontLibrary::FontLibrary()
	{
		bool success = true;
		success = success && FT_Init_FreeType(&library) == FT_Err_Ok;
		if (!LoadFallbackFont())
		{
			Log("Failed to load embeded fallback font", Logger::Warning);
		}
		if (!success)
			assert(false);
	}

	FontLibrary::~FontLibrary()
	{
		FT_Done_Face(fallbackFont);
		FT_Done_FreeType(library);
	}
}