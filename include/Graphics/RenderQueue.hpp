#pragma once
#include <Graphics/Mesh.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Font.hpp>
#include <Graphics/Material.hpp>

/*
	Represents a draw command that can be executed in a render queue
*/
class RenderQueueItem
{
public:
	virtual ~RenderQueueItem() = default;
};

// Most basic draw command that only contains a material, it's parameters and a world transform
class SimpleDrawCall : public RenderQueueItem
{
public:
	// The mesh to draw
	Mesh mesh;
	// Material to use
	Material mat;
	MaterialParameterSet params;
	// The world transform
	Transform worldTransform;
};

/*
	This class is a queue that collects draw commands
	each of these is stored together with their wanted render state.

	When Process is called, the commands are sorted and grouped, then sent to the graphics pipeline.
*/
class RenderQueue
{
public:
	RenderQueue(OpenGL* ogl, const RenderState& rs);
	void Process();
	void Draw(Transform worldTransform, Mesh m, Material mat, const MaterialParameterSet& params = MaterialParameterSet());
	void Draw(Transform worldTransform, Ref<class TextRes> text, Material mat, const MaterialParameterSet& params = MaterialParameterSet());

private:
	RenderState m_renderState;
	Vector<RenderQueueItem*> m_orderedCommands;
	class OpenGL* m_ogl;
};