#include "stdafx.h"
#include "Image.hpp"
#include "png.h"
// HAVE_STDDEF_H redefinition
#pragma warning(disable:4005)
#include "jpeglib.h"

namespace Graphics
{
	struct jpegErrorMgr : jpeg_error_mgr
	{
		jmp_buf jmpBuf;
	};

	static void jpegErrorExit(jpeg_common_struct* cinfo)
	{
		longjmp(static_cast<jpegErrorMgr*>(cinfo->err)->jmpBuf, 1);
	}

	static void jpegErrorReset(jpeg_common_struct* cinfo)
	{}

	static void jpegEmitMessage(jpeg_common_struct* cinfo, int msgLvl)
	{}

	static void jpegOutputMessage(jpeg_common_struct* cinfo)
	{}

	static void jpegFormatMessage(jpeg_common_struct* cinfo, char * buffer)
	{}

	void ImageRes::SetSize(Vector2i size)
	{
		m_size = size;
		clear();
		Allocate();
	}

	void ImageRes::ReSize(Vector2i size)
	{
		size_t new_DataLength = size.x * size.y;
		if (new_DataLength == 0)
		{
			return;
		}
		Colori* new_pData = new Colori[new_DataLength];

		for (int32 ix = 0; ix < size.x; ++ix)
		{
			for (int32 iy = 0; iy < size.y; ++iy)
			{
				int32 sampledX = ix * ((double)m_size.x / (double)size.x);
				int32 sampledY = iy * ((double)m_size.y / (double)size.y);
				new_pData[size.x * iy + ix] = m_pData[m_size.x * sampledY + sampledX];
			}
		}

		delete[] m_pData;
		m_pData = new_pData;
		m_size = size;
		m_nDataLength = m_size.x * m_size.y;
	}

	Vector2i ImageRes::GetSize() const
	{
		return m_size;
	}

	Colori* ImageRes::GetBits()
	{
		return m_pData;
	}

	const Colori* ImageRes::GetBits() const
	{
		return m_pData;
	}

	/**
	 * \throws std::runtime_error if failed to load image
	 */
	ImageRes::ImageRes(const String& assetPath)
	{
		if (!Load(assetPath))
		{
			Logf("Failed to load image %s", Logger::Severity::Error, assetPath);
			throw std::runtime_error("Failed to load image");
		}
	}

	ImageRes::ImageRes(Vector2i size)
	{
		SetSize(size);
	}

	ImageRes::~ImageRes()
	{
		clear();
	}

	bool ImageRes::Load(const String& fullPath)
	{
		File f;
		if (!f.OpenRead(fullPath))
			return false;

		Buffer b;
		b.resize(f.GetSize());
		f.Read(b.data(), b.size());
		if (b.size() < 4)
			return false;

		// Check for PNG based on first 4 bytes
		if (*(uint32*)b.data() == (uint32&)"‰PNG")
			return load_png(b);
		else // jay-PEG ?
			return load_jpeg(b);
	}

	bool ImageRes::load_jpeg(Buffer& in)
	{
		/* This struct contains the JPEG decompression parameters and pointers to
			* working space (which is allocated as needed by the JPEG library).
			*/
		jpeg_decompress_struct cinfo;
		jpegErrorMgr jerr = {};
		jerr.reset_error_mgr = &jpegErrorReset;
		jerr.error_exit = &jpegErrorExit;
		jerr.emit_message = &jpegEmitMessage;
		jerr.format_message = &jpegFormatMessage;
		jerr.output_message = &jpegOutputMessage;
		cinfo.err = &jerr;

		// Return point for long jump
		if (setjmp(jerr.jmpBuf) == 0)
		{
			jpeg_create_decompress(&cinfo);
			jpeg_mem_src(&cinfo, in.data(), (uint32)in.size());
			(void) jpeg_read_header(&cinfo, TRUE);

			jpeg_start_decompress(&cinfo);
			int row_stride = cinfo.output_width * cinfo.output_components;
			JSAMPARRAY sample = (*cinfo.mem->alloc_sarray)
				((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

			Vector2i size = Vector2i(cinfo.output_width, cinfo.output_height);
			SetSize(size);
			Colori* pBits = GetBits();

			size_t pixelSize = cinfo.out_color_components;
			cinfo.out_color_space = JCS_RGB;

			while (cinfo.output_scanline < cinfo.output_height)
			{
				jpeg_read_scanlines(&cinfo, sample, 1);
				for (size_t i = 0; i < cinfo.output_width; i++)
				{
					memcpy(pBits + i, sample[0] + i * pixelSize, pixelSize);
					pBits[i].w = 0xFF;
				}

				pBits += size.x;
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			return true;
		}

		// If we get here, the loading of the jpeg failed
		return false;
	}

	bool ImageRes::load_png(Buffer& in)
	{
		png_image image;
		memset(&image, 0, (sizeof image));
		image.version = PNG_IMAGE_VERSION;

		if (png_image_begin_read_from_memory(&image, in.data(), in.size()) == 0)
			return false;

		image.format = PNG_FORMAT_RGBA;

		SetSize(Vector2i(image.width, image.height));
		Colori* pBuffer = GetBits();
		if (!pBuffer)
			return false;

		if ((image.width * image.height * 4) != PNG_IMAGE_SIZE(image))
			return false;

		if (png_image_finish_read(&image, nullptr, pBuffer, 0, nullptr) == 0)
			return false;

		png_image_free(&image);
		return true;
	}

	void ImageRes::Allocate()
	{
		m_nDataLength = m_size.x * m_size.y;
		if (m_nDataLength == 0)
			return;
		m_pData = new Colori[m_nDataLength];
	}

	void ImageRes::clear()
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
}
