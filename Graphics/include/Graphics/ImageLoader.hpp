#pragma once

namespace Graphics
{
	/*
		Static image loader
		Supports the following formats:
			- PNG (RGB8, RGBA8)
			- JPEG
	 */
	class ImageRes;

	class ImageLoader
	{
	public:
		static bool Load(shared_ptr<ImageRes> outPtr, const String& fullPath);
	};
}
