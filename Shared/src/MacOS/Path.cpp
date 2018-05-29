#include "stdafx.h"
#include "Path.hpp"
#include "Log.hpp"
#include "File.hpp"
#include "Math.hpp"

/*
	macOS version
*/
#include <mach-o/dyld.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
// Convenience
#define MAX_PATH PATH_MAX

char Path::sep = '/';

bool Path::CreateDir(const String& path)
{
	return mkdir(*path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}
bool Path::Delete(const String& path)
{
    return remove(*path) == 0;
}
bool Path::DeleteDir(const String& path)
{
	if(!ClearDir(path))
		return false;
    return rmdir(*path) == 0;
}
bool Path::Rename(const String& srcFile, const String& dstFile, bool overwrite)
{
	if(FileExists(*dstFile))
	{
		if(!overwrite)
			return false;
		if(Delete(*dstFile))
		{
			Logf("Failed to rename file, overwrite was true but the destination could not be removed", Logger::Warning);
			return false;
		}
	}
	return rename(*srcFile, *dstFile) == 0;
}
bool Path::Copy(const String& srcFile, const String& dstFile, bool overwrite)
{
    File src;
    if(!src.OpenRead(srcFile))
        return false;
    File dst;
    if(!dst.OpenWrite(dstFile))
        return false;

    size_t sizeMax = src.GetSize();
    size_t remaining = sizeMax;
    char buffer[8096];

    // Do a buffered copy
    while(remaining > 0)
    {
        size_t read = Math::Min(remaining, sizeof(buffer));
        if(src.Read(buffer, read) != read)
            return false;
        if(dst.Write(buffer, read) != read)
            return false;
        remaining -= read;
    }
    return true;
}
String Path::GetCurrentPath()
{
	char currDir[MAX_PATH];
	getcwd(currDir, MAX_PATH);
	return currDir;
}
String Path::GetExecutablePath()
{
	// Get name from pid
	char path[MAX_PATH];
	char real_path[MAX_PATH];

        uint32_t path_size = MAX_PATH;
        int found_path = _NSGetExecutablePath(path, &path_size);
	assert(found_path != -1);
        char* r = realpath(path, real_path);
        assert(r != NULL);

	return String(real_path);
}
String Path::GetTemporaryPath()
{
	char out[MAX_PATH];
	String curr = GetCurrentPath();
	sprintf(out, "%s/tempXXXXXX", *curr);
	mktemp(out);
	return out;
}
String Path::GetTemporaryFileName(const String& path, const String& prefix)
{
	char out[MAX_PATH];
	sprintf(out, "%s/%sXXXXXX", *path, *prefix);
	mktemp(out);
	return out;
}

bool Path::IsDirectory(const String& path)
{
	struct stat sb;
    if (stat(*path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;
	return false;
}

bool Path::FileExists(const String& path)
{
	return access(path.c_str(), F_OK) != -1;
}
String Path::Normalize(const String& path)
{
	char out[MAX_PATH];
	realpath(*path, out);
	for(uint32 i = 0; i < MAX_PATH; i++)
	{
		if(out[i] == '\\')
			out[i] = sep;
	}
	return out;
}
bool Path::IsAbsolute(const String& path)
{
	if(path.length() > 1 && path[0] == '/')
		return true;
	return false;
}

Vector<String> Path::GetSubDirs(const String& path)
{
    Vector<String> ret;
    DIR* dir = opendir(path.c_str());
    if(dir == nullptr)
        return ret;

    // Open first entry
    dirent* ent = readdir(dir);
    if(ent)
    {
        // Keep scanning files in this folder
        do
        {
            String filename = ent->d_name;

            if(filename == ".")
                continue;
            if(filename == "..")
                continue;

            if(ent->d_type == DT_DIR)
            {
                ret.Add(filename);
            }
        } while((ent = readdir(dir)));
    }
    return ret;
}
