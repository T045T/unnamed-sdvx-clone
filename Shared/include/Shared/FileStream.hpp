#pragma once
#include "Shared/BinaryStream.hpp"
#include "Shared/File.hpp"

class FileStreamBase : public BinaryStream
{
public:
	FileStreamBase()= default;
	FileStreamBase(File& file, bool isReading);
	void Seek(size_t pos) override;
	size_t Tell() const override;
	size_t GetSize() const override;
	File& GetFile() const;

protected:
	File* m_file = nullptr;
};

/* Stream that reads from a buffer */
class FileReader : public FileStreamBase
{
public:
	FileReader() = default;
	FileReader(File& file);
	size_t Serialize(void* data, size_t len) override;
};

/* Stream that writes to a buffer */
class FileWriter : public FileStreamBase
{
public:
	FileWriter() = default;
	FileWriter(File& file);
	size_t Serialize(void* data, size_t len) override;
};