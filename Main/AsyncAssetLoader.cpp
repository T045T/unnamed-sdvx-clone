#include "stdafx.h"
#include "AsyncAssetLoader.hpp"
#include "Application.hpp"
#include "Global.hpp"

struct AsyncTextureLoadOperation : AsyncLoadOperation
{
	Texture& target;
	Image image;

	AsyncTextureLoadOperation(Texture& target, const String& path)
		: target(target)
	{
		name = path;
	}

	bool AsyncLoad() override
	{
		image = g_application->LoadImage(name);
		return image != nullptr;
	}

	/**
	 * \throws std::runtime_error if failed to create TextureRes
	 */
	bool AsyncFinalize() override
	{
		target = make_shared<TextureRes>(image);
		return target != nullptr;
	}
};

struct AsyncMeshLoadOperation : AsyncLoadOperation
{
	Mesh& target;

	AsyncMeshLoadOperation(Mesh& target, const String& path)
		: target(target)
	{}

	bool AsyncLoad() override
	{
		/// TODO: No mesh loading yet
		return false;
	}

	bool AsyncFinalize() override
	{
		/// TODO: No mesh loading yet
		return false;
	}
};

struct AsyncMaterialLoadOperation : AsyncLoadOperation
{
	Material& target;

	AsyncMaterialLoadOperation(Material& target, const String& path)
		: target(target)
	{
		name = path;
	}

	bool AsyncLoad() override
	{
		return true;
	}

	bool AsyncFinalize() override
	{
		return (target = g_application->LoadMaterial(name)) != nullptr;
	}
};

struct AsyncWrapperOperation : AsyncLoadOperation
{
	IAsyncLoadable& target;

	AsyncWrapperOperation(IAsyncLoadable& target, const String& name)
		: target(target)
	{
		this->name = name;
	}

	bool AsyncLoad() override
	{
		return target.AsyncLoad();
	}

	bool AsyncFinalize() override
	{
		return target.AsyncFinalize();
	}
};

void AsyncAssetLoader::AddTexture(Texture& out, const String& path)
{
	loadables.push_back(make_unique<AsyncTextureLoadOperation>(out, path));
}

void AsyncAssetLoader::AddMesh(Mesh& out, const String& path)
{
	loadables.push_back(make_unique<AsyncMeshLoadOperation>(out, path));
}

void AsyncAssetLoader::AddMaterial(Material& out, const String& path)
{
	loadables.push_back(make_unique<AsyncMaterialLoadOperation>(out, path));
}

void AsyncAssetLoader::AddLoadable(IAsyncLoadable& loadable, const String& id /*= "unknown"*/)
{
	loadables.push_back(make_unique<AsyncWrapperOperation>(loadable, id));
}

bool AsyncAssetLoader::Load()
{
	bool success = true;
	for (auto& ld : loadables)
	{
		if (!ld->AsyncLoad())
		{
			Logf("[AsyncLoad] Load failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}
	return success;
}

bool AsyncAssetLoader::Finalize()
{
	bool success = true;
	for (auto& ld : loadables)
	{
		if (!ld->AsyncFinalize())
		{
			Logf("[AsyncLoad] Finalize failed on %s", Logger::Error, ld->name);
			success = false;
		}
	}

	// Clear state
	loadables.clear();

	return success;
}
