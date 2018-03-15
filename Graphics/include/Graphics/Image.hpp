#pragma once
#include <Graphics/ResourceTypes.hpp>

namespace Graphics
{
	/*
		RGBA8 image class
		The bits have the same layout as the Colori class
	*/
	class ImageRes
	{
	public:
		virtual ~ImageRes() = default;
		static std::shared_ptr<ImageRes> Create(const String& assetPath);
		static std::shared_ptr<ImageRes> Create(Vector2i size = Vector2i());
	public:
		virtual void SetSize(Vector2i size) = 0;
		virtual void ReSize(Vector2i size) = 0;
		virtual Vector2i GetSize() const = 0;
		virtual Colori* GetBits() = 0;
		virtual const Colori* GetBits() const = 0;
	};

	/*
		Sprite map
		Adding images to this will pack the image into a final image that contains all the added images
		After this the UV coordinates of these images can be asked for given and image index

		!! The packing is not optimal as the images are stacked for bottom to top and placed in columns based on their width
	*/
	class TextureRes;

	class SpriteMapRes
	{
	public:
		virtual ~SpriteMapRes() = default;
		static std::shared_ptr<SpriteMapRes> Create();
	public:
		virtual uint32 AddSegment(std::shared_ptr<ImageRes> image) = 0;
		virtual void Clear() = 0;
		virtual std::shared_ptr<ImageRes> GetImage() = 0;
		virtual std::shared_ptr<class TextureRes> GenerateTexture(class OpenGL* gl) = 0;
		virtual Recti GetCoords(uint32 nIndex) = 0;
	};

	typedef std::shared_ptr<ImageRes> Image;
	typedef std::shared_ptr<SpriteMapRes> SpriteMap;

	DEFINE_RESOURCE_TYPE(Image, ImageRes);

	DEFINE_RESOURCE_TYPE(SpriteMap, SpriteMapRes);
}
