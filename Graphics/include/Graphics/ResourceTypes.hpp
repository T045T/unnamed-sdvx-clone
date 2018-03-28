#pragma once

namespace Graphics
{
	/* Enum of all graphics resource types in this library */
	enum class ResourceType
	{
		Image = 0,
		SpriteMap,
		Texture,
		Framebuffer,
		Font,
		Mesh,
		Shader,
		Material,
		ParticleSystem,
		_Length
	};
}
