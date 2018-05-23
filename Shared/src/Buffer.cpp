#include "stdafx.h"
#include "Buffer.hpp"

Buffer::Buffer(size_t initialSize)
{
	resize(initialSize);
}

Buffer::Buffer(const char* string)
{
	const uint32 l = static_cast<uint32>(strlen(string));

	for (uint32 i = 0; i < l; i++)
		push_back(static_cast<uint8>(string[i]));
}

Buffer::Buffer(Buffer&& rhs) noexcept
{
	static_cast<vector<uint8>*>(this)->operator=(static_cast<Buffer&&>(rhs));
}

Buffer Buffer::Copy() const
{
	Buffer newBuffer;

	if (!empty())
	{
		newBuffer.resize(size());
		memcpy(newBuffer.data(), data(), size());
	}

	return static_cast<Buffer&&>(newBuffer);
}

Buffer& Buffer::operator=(Buffer&& rhs) noexcept
{
	Vector<uint8>::operator=(rhs);
	return *this;
}

CopyableBuffer& CopyableBuffer::operator=(const CopyableBuffer& rhs)
{
	static_cast<Vector<uint8>&>(*this) = rhs;
	return *this;
}

CopyableBuffer::CopyableBuffer(const CopyableBuffer& rhs)
{
	static_cast<Vector<uint8>&>(*this) = rhs;
}
