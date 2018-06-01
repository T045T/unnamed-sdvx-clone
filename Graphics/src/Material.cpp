#include "stdafx.h"
#include "Material.hpp"
#include "OpenGL.hpp"
#include "RenderQueue.hpp"

namespace Graphics
{
	const char* builtInShaderVariableNames[] =
	{
		"world",
		"proj",
		"camera",
		"billboard",
		"viewport",
		"aspectRatio",
		"time",
	};

	class BuiltInShaderVariableMap : public Map<String, BuiltInShaderVariable>
	{
	public:
		BuiltInShaderVariableMap()
		{
			for (int32 i = 0; i < SV__BuiltInEnd; i++)
			{
				Add(builtInShaderVariableNames[i], static_cast<BuiltInShaderVariable>(i));
			}
		}
	};

	BuiltInShaderVariableMap builtInShaderVariableMap;

	// Defined in Shader.cpp
	extern uint32 shaderStageMap[];

	template <>
	void MaterialRes::BindShaderVar<Vector4>(uint32 shader, uint32 loc, const Vector4& obj)
	{
		glProgramUniform4fv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<Vector3>(uint32 shader, uint32 loc, const Vector3& obj)
	{
		glProgramUniform3fv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<Vector2>(uint32 shader, uint32 loc, const Vector2& obj)
	{
		glProgramUniform2fv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<float>(uint32 shader, uint32 loc, const float& obj)
	{
		glProgramUniform1fv(shader, loc, 1, &obj);
	}

	template <>
	void MaterialRes::BindShaderVar<Colori>(uint32 shader, uint32 loc, const Colori& obj)
	{
		Color c = obj;
		glProgramUniform4fv(shader, loc, 1, &c.x);
	}

	template <>
	void MaterialRes::BindShaderVar<Vector4i>(uint32 shader, uint32 loc, const Vector4i& obj)
	{
		glProgramUniform4iv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<Vector3i>(uint32 shader, uint32 loc, const Vector3i& obj)
	{
		glProgramUniform3iv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<Vector2i>(uint32 shader, uint32 loc, const Vector2i& obj)
	{
		glProgramUniform2iv(shader, loc, 1, &obj.x);
	}

	template <>
	void MaterialRes::BindShaderVar<int32>(uint32 shader, uint32 loc, const int32& obj)
	{
		glProgramUniform1iv(shader, loc, 1, &obj);
	}

	template <>
	void MaterialRes::BindShaderVar<Transform>(uint32 shader, uint32 loc, const Transform& obj)
	{
		glProgramUniformMatrix4fv(shader, loc, 1, GL_FALSE, obj.mat);
	}

	MaterialRes::MaterialRes(shared_ptr<OpenGL> gl)
		: m_gl(std::move(gl))
	{
		glGenProgramPipelines(1, &m_pipeline);
	}

	/**
	 * \throws std::runtime_error if fails to load vertex/material shader
	 * \throws std::runtime_error if fails to create shader
	 */
	MaterialRes::MaterialRes(shared_ptr<OpenGL> gl, const String& vsPath, const String& fsPath)
		: MaterialRes(gl)
	{
		AssignShader(ShaderType::Vertex, make_shared<ShaderRes>(gl, ShaderType::Vertex, vsPath));
		AssignShader(ShaderType::Fragment, make_shared<ShaderRes>(gl, ShaderType::Fragment, fsPath));
#if _DEBUG

		m_debugNames[static_cast<size_t>(ShaderType::Vertex)] = vsPath;
		m_debugNames[static_cast<size_t>(ShaderType::Fragment)] = fsPath;
#endif

		if (!m_shaders[static_cast<size_t>(ShaderType::Vertex)])
		{
			Logf("Failed to load vertex shader for material from %s", Logger::Error, vsPath);
			throw std::runtime_error("Failed to load vertex shader");
		}

		if (!m_shaders[static_cast<size_t>(ShaderType::Fragment)])
		{
			Logf("Failed to load fragment shader for material from %s", Logger::Error, fsPath);
			throw std::runtime_error("Failed to load material shader");
		}
	}

	MaterialRes::~MaterialRes()
	{
		glDeleteProgramPipelines(1, &m_pipeline);
	}

	void MaterialRes::AssignShader(ShaderType t, Shader _shader)
	{
		const auto shader = std::move(_shader);
		m_shaders[static_cast<size_t>(t)] = shader;

		uint32 handle = shader->Handle();

#ifdef _DEBUG
		Logf("Listing shader uniforms for %s", Logger::Info, shader->GetOriginalName());
#endif // _DEBUG

		int32 numUniforms;
		glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniforms);
		for (int32 i = 0; i < numUniforms; i++)
		{
			char name[64];
			int32 nameLen, size;
			uint32 type;
			glGetActiveUniform(handle, i, sizeof(name), &nameLen, &size, &type, name);
			const uint32 loc = glGetUniformLocation(handle, name);

			// Select type
			String typeName = "Unknown";
			if (type == GL_SAMPLER_2D)
			{
				typeName = "Sampler2D";
				if (!m_textureIDs.Contains(name))
					m_textureIDs.Add(name, m_textureID++);
			}
			else if (type == GL_FLOAT_MAT4)
				typeName = "Transform";
			else if (type == GL_FLOAT_VEC4)
				typeName = "Vector4";
			else if (type == GL_FLOAT_VEC3)
				typeName = "Vector3";
			else if (type == GL_FLOAT_VEC2)
				typeName = "Vector2";
			else if (type == GL_FLOAT)
				typeName = "Float";

			// Built in variable?
			uint32 targetID = 0;
			if (builtInShaderVariableMap.Contains(name))
				targetID = builtInShaderVariableMap[name];
			else
			{
				if (m_mappedParameters.Contains(name))
					targetID = m_mappedParameters[name];
				else
					targetID = m_mappedParameters.Add(name, m_userID++);
			}

			(void) m_boundParameters.FindOrAdd(targetID).Add(BoundParameterInfo(t, type, loc));

#ifdef _DEBUG
			Logf("Uniform [%d, loc=%d, %s] = %s", Logger::Info,
				i, loc, Utility::Sprintf("Unknown [%d]", type), name);
#endif // _DEBUG
		}

		glUseProgramStages(m_pipeline, shaderStageMap[static_cast<size_t>(t)], shader->Handle());
	}

	void MaterialRes::Bind(const RenderState& rs, const MaterialParameterSet& params)
	{
#if _DEBUG
		bool reloadedShaders = false;
		for (uint32 i = 0; i < 3; i++)
		{
			if (m_shaders[i] && m_shaders[i]->UpdateHotReload())
			{
				reloadedShaders = true;
			}
		}

		// Regenerate parameter map
		if (reloadedShaders)
		{
			Log("Reloading material", Logger::Info);
			m_boundParameters.clear();
			m_textureIDs.clear();
			m_mappedParameters.clear();
			m_userID = SV_User;
			m_textureID = 0;
			for (uint32 i = 0; i < 3; i++)
			{
				if (m_shaders[i])
					AssignShader(ShaderType(i), m_shaders[i]);
			}
		}
#endif

		// Bind renderstate variables
		BindAll(SV_Proj, rs.projectionTransform);
		BindAll(SV_Camera, rs.cameraTransform);
		BindAll(SV_Viewport, rs.viewportSize);
		BindAll(SV_AspectRatio, rs.aspectRatio);
		Transform billboard = CameraMatrix::BillboardMatrix(rs.cameraTransform);
		BindAll(SV_BillboardMatrix, billboard);
		BindAll(SV_Time, rs.time);

		// Bind parameters
		BindParameters(params, rs.worldTransform);

		BindToContext();
	}

	void MaterialRes::BindParameters(const MaterialParameterSet& params, const Transform& worldTransform)
	{
		BindAll(SV_World, worldTransform);
		for (auto p : params)
		{
			switch (p.second.parameterType)
			{
			case GL_INT:
				BindAll(p.first, p.second.Get<int>());
				break;
			case GL_FLOAT:
				BindAll(p.first, p.second.Get<float>());
				break;
			case GL_INT_VEC2:
				BindAll(p.first, p.second.Get<Vector2i>());
				break;
			case GL_INT_VEC3:
				BindAll(p.first, p.second.Get<Vector3i>());
				break;
			case GL_INT_VEC4:
				BindAll(p.first, p.second.Get<Vector4i>());
				break;
			case GL_FLOAT_VEC2:
				BindAll(p.first, p.second.Get<Vector2>());
				break;
			case GL_FLOAT_VEC3:
				BindAll(p.first, p.second.Get<Vector3>());
				break;
			case GL_FLOAT_VEC4:
				BindAll(p.first, p.second.Get<Vector4>());
				break;
			case GL_FLOAT_MAT4:
				BindAll(p.first, p.second.Get<Transform>());
				break;
			case GL_SAMPLER_2D:
			{
				uint32* textureUnit = m_textureIDs.Find(p.first);
				if (!textureUnit)
				{
					/// TODO: Add print once mechanism for these kind of errors
					//Logf("Texture not found \"%s\"", Logger::Warning, p.first);
					break;
				}
				uint32 texture = p.second.Get<int32>();

				// Bind the texture
				glActiveTexture(GL_TEXTURE0 + *textureUnit);
				glBindTexture(GL_TEXTURE_2D, texture);

				// Bind sampler
				BindAll<int32>(p.first, *textureUnit);
				break;
			}
			default:
				assert(false);
			}
		}
	}

	void MaterialRes::BindToContext() const
	{
		// Bind pipeline to context
		glBindProgramPipeline(m_pipeline);
	}

	BoundParameterInfo* MaterialRes::GetBoundParameters(const String& name, uint32& count)
	{
		uint32* mappedID = m_mappedParameters.Find(name);
		if (!mappedID)
			return nullptr;
		return GetBoundParameters(static_cast<BuiltInShaderVariable>(*mappedID), count);
	}

	BoundParameterInfo* MaterialRes::GetBoundParameters(BuiltInShaderVariable bsv, uint32& count)
	{
		auto l = m_boundParameters.Find(bsv);
		if (!l)
			return nullptr;
		else
		{
			count = static_cast<uint32>(l->size());
			return l->data();
		}
	}

	void MaterialParameterSet::SetParameter(const String& name, int sc)
	{
		Add(name, MaterialParameter::Create(sc, GL_INT));
	}

	void MaterialParameterSet::SetParameter(const String& name, float sc)
	{
		Add(name, MaterialParameter::Create(sc, GL_FLOAT));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector4& vec)
	{
		Add(name, MaterialParameter::Create(vec, GL_FLOAT_VEC4));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Colori& color)
	{
		Add(name, MaterialParameter::Create(Color(color), GL_FLOAT_VEC4));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector2& vec2)
	{
		Add(name, MaterialParameter::Create(vec2, GL_FLOAT_VEC2));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector3& vec3)
	{
		Add(name, MaterialParameter::Create(vec3, GL_FLOAT_VEC3));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Transform& tf)
	{
		Add(name, MaterialParameter::Create(tf, GL_FLOAT_MAT4));
	}

	void MaterialParameterSet::SetParameter(const String& name, std::shared_ptr<class TextureRes> tex)
	{
		Add(name, MaterialParameter::Create(tex->Handle(), GL_SAMPLER_2D));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector2i& vec2)
	{
		Add(name, MaterialParameter::Create(vec2, GL_INT_VEC2));
	}
}
