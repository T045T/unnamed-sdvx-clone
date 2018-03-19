#include "stdafx.h"
#include "RenderQueue.hpp"
#include <utility>
#include "OpenGL.hpp"
using Utility::DynCast;

namespace Graphics
{
	RenderQueue::RenderQueue(shared_ptr<OpenGL> gl, RenderState rs)
		: m_renderState(std::move(rs)),
		  m_gl(std::move(gl))
	{}

	RenderQueue::RenderQueue(RenderQueue&& other)
	{
		m_gl = other.m_gl;
		other.m_gl = nullptr;
		m_orderedCommands = move(other.m_orderedCommands);
		m_renderState = other.m_renderState;
	}

	RenderQueue& RenderQueue::operator=(RenderQueue&& other)
	{
		Clear();
		m_gl = other.m_gl;
		other.m_gl = nullptr;
		m_orderedCommands = move(other.m_orderedCommands);
		m_renderState = other.m_renderState;
		return *this;
	}

	RenderQueue::~RenderQueue()
	{
		Clear();
	}

	void RenderQueue::Process(bool clearQueue)
	{
		assert(m_gl);

		bool scissorEnabled = false;
		bool blendEnabled = false;
		MaterialBlendMode activeBlendMode = (MaterialBlendMode)-1;

		Set<Material> initializedShaders;
		Mesh currentMesh;
		Material currentMaterial;

		// Create a new list of items
		for (RenderQueueItem* item : m_orderedCommands)
		{
			auto SetupMaterial = [&](Material& mat, MaterialParameterSet& params)
			{
				// Only bind params if material is already bound to context
				if (currentMaterial == mat)
					mat->BindParameters(params, m_renderState.worldTransform);
				else
				{
					if (initializedShaders.Contains(mat))
					{
						// Only bind params and rebind
						mat->BindParameters(params, m_renderState.worldTransform);
						mat->BindToContext();
						currentMaterial = mat;
					}
					else
					{
						mat->Bind(m_renderState, params);
						initializedShaders.Add(mat);
						currentMaterial = mat;
					}
				}

				// Setup Render state for transparent object
				if (mat->opaque)
				{
					if (blendEnabled)
					{
						glDisable(GL_BLEND);
						blendEnabled = false;
					}
				}
				else
				{
					if (!blendEnabled)
					{
						glEnable(GL_BLEND);
						blendEnabled = true;
					}
					if (activeBlendMode != mat->blendMode)
					{
						switch (mat->blendMode)
						{
						case MaterialBlendMode::Normal:
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							break;
						case MaterialBlendMode::Additive:
							glBlendFunc(GL_SRC_ALPHA, GL_ONE);
							break;
						case MaterialBlendMode::Multiply:
							glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
							break;
						}
					}
				}
			};

			// Draw mesh helper
			auto DrawOrRedrawMesh = [&](Mesh& mesh)
			{
				if (currentMesh == mesh)
					mesh->Redraw();
				else
				{
					mesh->Draw();
					currentMesh = mesh;
				}
			};

			if (DynCast<SimpleDrawCall>(item))
			{
				SimpleDrawCall* sdc = (SimpleDrawCall*)item;
				m_renderState.worldTransform = sdc->worldTransform;
				SetupMaterial(sdc->mat, sdc->params);

				// Check if scissor is enabled
				bool useScissor = (sdc->scissorRect.size.x >= 0);
				if (useScissor)
				{
					// Apply scissor
					if (!scissorEnabled)
					{
						glEnable(GL_SCISSOR_TEST);
						scissorEnabled = true;
					}
					float scissorY = m_renderState.viewportSize.y - sdc->scissorRect.Bottom();
					glScissor((int32)sdc->scissorRect.Left(), (int32)scissorY,
							(int32)sdc->scissorRect.size.x, (int32)sdc->scissorRect.size.y);
				}
				else
				{
					if (scissorEnabled)
					{
						glDisable(GL_SCISSOR_TEST);
						scissorEnabled = false;
					}
				}

				DrawOrRedrawMesh(sdc->mesh);
			}
			else if (DynCast<PointDrawCall>(item))
			{
				if (scissorEnabled)
				{
					// Disable scissor
					glDisable(GL_SCISSOR_TEST);
					scissorEnabled = false;
				}

				PointDrawCall* pdc = (PointDrawCall*)item;
				m_renderState.worldTransform = Transform();
				SetupMaterial(pdc->mat, pdc->params);
				PrimitiveType pt = pdc->mesh->GetPrimitiveType();
				if (pt >= PrimitiveType::LineList && pt <= PrimitiveType::LineStrip)
				{
					glLineWidth(pdc->size);
				}
				else
				{
					glPointSize(pdc->size);
				}

				DrawOrRedrawMesh(pdc->mesh);
			}
		}

		// Disable all states that were on
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);

		if (clearQueue)
		{
			Clear();
		}
	}

	void RenderQueue::Clear()
	{
		// Cleanup the list of items
		for (RenderQueueItem* item : m_orderedCommands)
		{
			delete item;
		}
		m_orderedCommands.clear();
	}

	void RenderQueue::Draw(Transform worldTransform, Mesh m, Material mat, const MaterialParameterSet& params)
	{
		SimpleDrawCall* sdc = new SimpleDrawCall();
		sdc->mat = mat;
		sdc->mesh = m;
		sdc->params = params;
		sdc->worldTransform = worldTransform;
		m_orderedCommands.push_back(sdc);
	}

	void RenderQueue::Draw(Transform worldTransform, std::shared_ptr<class TextRes> text, Material mat,
							const MaterialParameterSet& params)
	{
		SimpleDrawCall* sdc = new SimpleDrawCall();
		sdc->mat = mat;
		sdc->mesh = text->get_mesh();
		sdc->params = params;
		// Set Font texture map
		sdc->params.SetParameter("mainTex", text->get_texture());
		sdc->worldTransform = worldTransform;
		m_orderedCommands.push_back(sdc);
	}

	void RenderQueue::DrawScissored(Rect scissor, Transform worldTransform, Mesh m, Material mat,
									const MaterialParameterSet& params /*= MaterialParameterSet()*/)
	{
		SimpleDrawCall* sdc = new SimpleDrawCall();
		sdc->mat = mat;
		sdc->mesh = m;
		sdc->params = params;
		sdc->worldTransform = worldTransform;
		sdc->scissorRect = scissor;
		m_orderedCommands.push_back(sdc);
	}

	void RenderQueue::DrawScissored(Rect scissor, Transform worldTransform, std::shared_ptr<class TextRes> text,
									Material mat, const MaterialParameterSet& params /*= MaterialParameterSet()*/)
	{
		SimpleDrawCall* sdc = new SimpleDrawCall();
		sdc->mat = mat;
		sdc->mesh = text->get_mesh();
		sdc->params = params;
		// Set Font texture map
		sdc->params.SetParameter("mainTex", text->get_texture());
		sdc->worldTransform = worldTransform;
		sdc->scissorRect = scissor;
		m_orderedCommands.push_back(sdc);
	}

	void RenderQueue::DrawPoints(Mesh m, Material mat, const MaterialParameterSet& params, float pointSize)
	{
		PointDrawCall* pdc = new PointDrawCall();
		pdc->mat = mat;
		pdc->mesh = m;
		pdc->params = params;
		pdc->size = pointSize;
		m_orderedCommands.push_back(pdc);
	}

	// Initializes the simple draw call structure
	SimpleDrawCall::SimpleDrawCall()
		: scissorRect(Vector2(), Vector2(-1))
	{ }
}
