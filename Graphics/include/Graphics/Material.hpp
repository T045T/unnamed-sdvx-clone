#pragma once
#include <Graphics/Shader.hpp>
#include <Graphics/RenderState.hpp>

namespace Graphics
{
	/* A single parameter that is set for a material */
	struct MaterialParameter
	{
		CopyableBuffer parameterData;
		uint32 parameterType;

		template <typename T>
		static MaterialParameter Create(const T& obj, uint32 type)
		{
			MaterialParameter r;
			r.Bind(obj);
			r.parameterType = type;
			return r;
		}

		template <typename T>
		void Bind(const T& obj)
		{
			parameterData.resize(sizeof(T));
			memcpy(parameterData.data(), &obj, sizeof(T));
		}

		template <typename T>
		const T& Get()
		{
			assert(sizeof(T) == parameterData.size());
			return *(T*)parameterData.data();
		}

		bool operator==(const MaterialParameter& other) const
		{
			if (parameterType != other.parameterType)
				return false;
			if (parameterData.size() != other.parameterData.size())
				return false;
			return memcmp(parameterData.data(), other.parameterData.data(), parameterData.size()) == 0;
		}
	};

	/*
		A list of parameters that is set for a material
		use SetParameter(name, param) to set any parameter by name
	*/
	class MaterialParameterSet : public Map<String, MaterialParameter>
	{
	public:
		using Map<String, MaterialParameter>::Map;
		void SetParameter(const String& name, int sc);
		void SetParameter(const String& name, float sc);
		void SetParameter(const String& name, const Vector4& vec);
		void SetParameter(const String& name, const Colori& color);
		void SetParameter(const String& name, const Vector2& vec2);
		void SetParameter(const String& name, const Vector3& vec3);
		void SetParameter(const String& name, const Vector2i& vec2);
		void SetParameter(const String& name, const Transform& tf);
		void SetParameter(const String& name, std::shared_ptr<class TextureRes> tex);
	};

	enum class MaterialBlendMode
	{
		Normal,
		Additive,
		Multiply,
	};

	// Defines build in shader variables
	enum BuiltInShaderVariable
	{
		SV_World = 0,
		SV_Proj,
		SV_Camera,
		SV_BillboardMatrix,
		SV_Viewport,
		SV_AspectRatio,
		SV_Time,
		SV__BuiltInEnd,
		SV_User = 0x100,
		// Start defining user variables here
	};

	struct BoundParameterInfo
	{
		BoundParameterInfo(ShaderType shaderType, uint32 paramType, uint32 location)
			: shaderType(shaderType), paramType(paramType), location(location)
		{ }

		ShaderType shaderType;
		uint32 paramType;
		uint32 location;
	};

	/*
		Abstracts the use of shaders/uniforms/pipelines into a single interface class
	*/
	class MaterialRes
	{
	public:
		MaterialRes(shared_ptr<OpenGL> gl);
		MaterialRes(shared_ptr<OpenGL> gl, const String& vsPath, const String& fsPath);
		~MaterialRes();

		bool opaque = true;
		MaterialBlendMode blendMode = MaterialBlendMode::Normal;

		void AssignShader(ShaderType t, Shader _shader);
		void Bind(const RenderState& rs, const MaterialParameterSet& params = MaterialParameterSet());

		// Only binds parameters to the current shader
		void BindParameters(const MaterialParameterSet& params, const Transform& worldTransform);

		// Bind only shaders/pipeline to context
		void BindToContext() const;

		template <typename T>
		void BindAll(const String& name, const T& obj);

		template <typename T>
		void BindAll(BuiltInShaderVariable bsv, const T& obj);

		template <typename T>
		void BindShaderVar(uint32 shader, uint32 loc, const T& obj);

	private:
		shared_ptr<OpenGL> m_gl;
		Shader m_shaders[3];
#if _DEBUG
		String m_debugNames[3];
#endif
		uint32 m_pipeline;
		Map<uint32, Vector<BoundParameterInfo>> m_boundParameters;
		Map<String, uint32> m_mappedParameters;
		Map<String, uint32> m_textureIDs;
		uint32 m_userID = SV_User;
		uint32 m_textureID = 0;

		BoundParameterInfo* GetBoundParameters(const String& name, uint32& count);
		BoundParameterInfo* GetBoundParameters(BuiltInShaderVariable bsv, uint32& count);
	};

	template <typename T>
	void MaterialRes::BindAll(const String& name, const T& obj)
	{
		uint32 num = 0;
		BoundParameterInfo* bp = GetBoundParameters(name, num);
		for (uint32 i = 0; bp && i < num; i++)
		{
			BindShaderVar<T>(m_shaders[(size_t)bp[i].shaderType]->Handle(), bp[i].location, obj);
		}
	}

	template <typename T>
	void MaterialRes::BindAll(BuiltInShaderVariable bsv, const T& obj)
	{
		uint32 num = 0;
		BoundParameterInfo* bp = GetBoundParameters(bsv, num);
		for (uint32 i = 0; bp && i < num; i++)
		{
			BindShaderVar<T>(m_shaders[(size_t)bp[i].shaderType]->Handle(), bp[i].location, obj);
		}
	}

	template <typename T>
	void MaterialRes::BindShaderVar(uint32 shader, uint32 loc, const T& obj)
	{
		static_assert(sizeof(T) != 0, "Incompatible shader uniform type");
	}

	typedef shared_ptr<MaterialRes> Material;
}
