#pragma once
#include <Graphics/OpenGL.hpp>

namespace Graphics
{
	/* Enum of supported shader types */
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Geometry
	};

	/*
		A single unlinked OpenGL shader
	*/
	class ShaderRes
	{
	public:
		ShaderRes(shared_ptr<OpenGL> gl, ShaderType type, const String& assetPath);
		~ShaderRes();

		static void Unbind(shared_ptr<OpenGL> gl, ShaderType type);
		friend class OpenGL;

		// Tries to hot-reload the shader program, only works if _DEBUG is defined
		// returns true if the program was changed and thus the handle value also changed
		bool UpdateHotReload();
		void Bind();
		bool IsBound() const;
		uint32 GetLocation(const String& name) const;

		uint32 operator[](const char* name) const
		{
			return GetLocation(name);
		}

		uint32 Handle() const;
		String GetOriginalName() const;

	private:
		ShaderType m_type;
		uint32 m_prog;
		shared_ptr<OpenGL> m_gl;

		String m_sourcePath;

		// Hot Reload detection on windows
#ifdef _WIN32
		HANDLE m_changeNotification = INVALID_HANDLE_VALUE;
		uint64 m_lwt = -1;
#endif

		bool LoadProgram(uint32& programOut);
		void setup_change_handler();

		void BindUniform(uint32 loc, const Transform& mat) const;
		void BindUniformVec2(uint32 loc, const Vector2& v) const;
		void BindUniformVec3(uint32 loc, const Vector3& v) const;
		void BindUniformVec4(uint32 loc, const Vector4& v) const;
		void BindUniform(uint32 loc, int i) const;
		void BindUniform(uint32 loc, float i) const;
		void BindUniformArray(uint32 loc, const Transform* mat, size_t count) const;
		void BindUniformArray(uint32 loc, const Vector2* v2, size_t count) const;
		void BindUniformArray(uint32 loc, const Vector3* v3, size_t count) const;
		void BindUniformArray(uint32 loc, const Vector4* v4, size_t count) const;
		void BindUniformArray(uint32 loc, const float* i, size_t count) const;
		void BindUniformArray(uint32 loc, const int* i, size_t count) const;
	};

	typedef shared_ptr<ShaderRes> Shader;
}
