#include "stdafx.h"
#include "Image.hpp"
#include "Texture.hpp"
#include <set>

namespace Graphics
{
	static uint32 maxHeight = 1024;
	static uint32 border = 2;

	SpriteMapRes::SpriteMapRes()
	{
		m_image = make_shared<ImageRes>();
	}

	SpriteMapRes::~SpriteMapRes()
	{
		clear();
	}

	uint32 SpriteMapRes::AddSegment(shared_ptr<ImageRes> image)
	{
		// Create a new segment
		uint32 nI = (uint32)m_segments.size();
		Segment* pCurrentSegment = m_segments.Add(new Segment());
		pCurrentSegment->coords.size = image->GetSize();

		// Get a category that has space
		Category& cat = AssignCategory(image->GetSize());

		// Set offset for current segment
		pCurrentSegment->coords.pos = cat.offset;
		// Add size offset in category
		cat.offset.y += pCurrentSegment->coords.size.y + 1;

		// Copy image data
		CopySubImage(m_image.get(), image.get(), pCurrentSegment->coords.pos);

		// Add segment to this category
		cat.segments.push_back(nI);
		return nI;
	}

	void SpriteMapRes::clear()
	{
		for (auto s : m_segments)
			delete s;

		m_segments.clear();
		m_widths.clear();
		m_categoryByWidth.clear();
		m_usedSize = 0;
		m_image->SetSize(Vector2i(0));
	}

	shared_ptr<TextureRes> SpriteMapRes::GenerateTexture()
	{
		auto tex = make_shared<TextureRes>(m_image);
		tex->SetWrap(TextureWrap::Clamp, TextureWrap::Clamp);
		return tex;
	}

	Recti SpriteMapRes::GetCoords(uint32 nIndex)
	{
		assert(nIndex < m_segments.size());
		return m_segments[nIndex]->coords;
	}

	Category& SpriteMapRes::AssignCategory(Vector2i requestedSize)
	{
		int32 mostSpace = 0;
		Category* dstCat = nullptr;

		while (true)
		{
			auto range = m_categoryByWidth.equal_range(requestedSize.x);
			// Find a suitable category first
			for (auto it = range.first; it != range.second; it++)
			{
				Category& cat = m_widths[it->second];
				int32 remainingY = m_image->GetSize().y - cat.offset.y;
				if (remainingY > requestedSize.y)
				{
					// This category is OK
					mostSpace = remainingY;
					dstCat = &cat;
					break;
				}
			}

			// Create a new category if required
			if (!dstCat)
			{
				int32 remainingX = m_image->GetSize().x - m_usedSize;
				// Use horizontal space to add another collumn
				//	if height of image is big enough
				if (m_image->GetSize().y >= requestedSize.y && remainingX >= requestedSize.x)
				{
					Category& cat = m_widths.Add();
					cat.width = requestedSize.x;
					cat.offset = Vector2i(m_usedSize, 0);
					m_categoryByWidth.insert(std::make_pair(cat.width, (uint32)m_widths.size() - 1));
					m_usedSize += cat.width + 1;
				}
				else
				{
					int32 remainingX = (m_image->GetSize().x - m_usedSize);

					// Resize image
					int32 largestDim = Math::Max(m_usedSize + requestedSize.x,
						Math::Max(m_image->GetSize().y, requestedSize.y));
					int32 targetSize = (int32)pow(2, ceil(log(largestDim) / log(2)));
					if (m_image->GetSize().x != targetSize)
					{
						// Resize image
						auto newImage = make_shared<ImageRes>(Vector2i(targetSize));
						// Copy old image into new image
						if (m_image->GetSize().x > 0)
							CopySubImage(newImage.get(), m_image.get(), Vector2i());
						m_image = std::move(newImage);
					}
				}
			}
			else
			{
				break;
			}
		}

		return *dstCat;
	}

	void SpriteMapRes::CopySubImage(ImageRes* dst, ImageRes* src, Vector2i dstPos)
	{
		Vector2i dstSize = dst->GetSize();
		Vector2i srcSize = src->GetSize();

		Colori* pSrc = src->GetBits();
		uint32 nDstPitch = dst->GetSize().x;
		Colori* pDst = dst->GetBits() + dstPos.x + dstPos.y * nDstPitch;
		for (uint32 y = 0; y < (uint32)srcSize.y; y++)
		{
			for (uint32 x = 0; x < (uint32)srcSize.x; x++)
			{
				*pDst = *pSrc;
				pSrc++;
				pDst++;
			}
			pDst += (nDstPitch - srcSize.x);
		}
	}
}
