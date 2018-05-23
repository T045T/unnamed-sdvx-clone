#pragma once
#include "OpenGL.hpp"

namespace Graphics
{
	/*
		RGBA8 image class
		The bits have the same layout as the Colori class
	*/
	class ImageRes
	{
	public:
		ImageRes(const String& assetPath);
		ImageRes(Vector2i size = Vector2i());
		~ImageRes();

		void SetSize(Vector2i size);
		void ReSize(Vector2i size);
		Vector2i GetSize() const;
		Colori* GetBits();
		const Colori* GetBits() const;

	private:
		Vector2i m_size;
		Colori* m_pData = nullptr;
		size_t m_nDataLength;

		bool Load(const String& fullPath);
		bool load_jpeg(Buffer& in);
		bool load_png(Buffer& in);
		void Allocate();
		void clear();
	};

	/*
		Sprite map
		Adding images to this will pack the image into a final image that contains all the added images
		After this the UV coordinates of these images can be asked for given and image index

		!! The packing is not optimal as the images are stacked for bottom to top and placed in columns based on their width
	*/
	class TextureRes;

	struct Category
	{
		uint32 width = 0;
		Vector2i offset;
		Vector<uint32> segments;
	};

	struct Segment
	{
		Recti coords;
	};

	class SpriteMapRes
	{
	public:
		SpriteMapRes();
		~SpriteMapRes();

		uint32 AddSegment(shared_ptr<ImageRes> image);
		void clear();
		shared_ptr<class TextureRes> GenerateTexture();
		Recti GetCoords(uint32 nIndex);

	private:
		// The image that contains the current data
		shared_ptr<ImageRes> m_image;

		// Used size over the X axis
		int32 m_usedSize = 0;

		// Linear index of al segements
		Vector<Segment*> m_segments;
		Vector<Category> m_widths;
		std::multimap<uint32, uint32> m_categoryByWidth;

		Category& AssignCategory(Vector2i requestedSize);
		void CopySubImage(ImageRes* dst, ImageRes* src, Vector2i dstPos);
	};

	typedef shared_ptr<ImageRes> Image;
	typedef shared_ptr<SpriteMapRes> SpriteMap;
}
