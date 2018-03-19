#include "stdafx.h"
#include "Shader.hpp"
#include <Graphics/ResourceManagers.hpp>
#include "OpenGL.hpp"

namespace Graphics
{
	uint32 typeMap[] =
	{
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
		GL_GEOMETRY_SHADER,
	};
	uint32 shaderStageMap[] =
	{
		GL_VERTEX_SHADER_BIT,
		GL_FRAGMENT_SHADER_BIT,
		GL_GEOMETRY_SHADER_BIT,
	};

	class Shader_Impl : public ShaderRes
	{
		ShaderType m_type;
		uint32 m_prog;
		shared_ptr<OpenGL> m_gl;

		String m_sourcePath;

		// Hot Reload detection on windows
#ifdef _WIN32
		HANDLE m_changeNotification = INVALID_HANDLE_VALUE;
		uint64 m_lwt = -1;
#endif
	public:
		Shader_Impl(shared_ptr<OpenGL> gl)
			: m_gl(std::move(gl))
		{ }

		~Shader_Impl()
		{
			// Cleanup OpenGL resource
			if (glIsProgram(m_prog))
			{
				glDeleteProgram(m_prog);
			}

#ifdef _WIN32
			// Close change notification handle
			if (m_changeNotification != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_changeNotification);
			}
#endif
		}

		void SetupChangeHandler()
		{
#ifdef _WIN32
			if (m_changeNotification != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_changeNotification);
				m_changeNotification = INVALID_HANDLE_VALUE;
			}

			String rootFolder = Path::RemoveLast(m_sourcePath);
			m_changeNotification = FindFirstChangeNotificationA(*rootFolder, false, FILE_NOTIFY_CHANGE_LAST_WRITE);
#endif
		}

		bool LoadProgram(uint32& programOut)
		{
			File in;
			if (!in.OpenRead(m_sourcePath))
				return false;

			String sourceStr;
			sourceStr.resize(in.GetSize());
			if (sourceStr.empty())
				return false;

			in.Read(&sourceStr.front(), sourceStr.size());

			const char* pChars = *sourceStr;
			programOut = glCreateShaderProgramv(typeMap[static_cast<size_t>(m_type)], 1, &pChars);
			if (programOut == 0)
				return false;

			int nStatus = 0;
			glGetProgramiv(programOut, GL_LINK_STATUS, &nStatus);
			if (nStatus == 0)
			{
				static char infoLogBuffer[2048];
				int s = 0;
				glGetProgramInfoLog(programOut, sizeof(infoLogBuffer), &s, infoLogBuffer);

				Logf("Shader program compile log for %s: %s", Logger::Error, m_sourcePath, infoLogBuffer);
				return false;
			}

			// Shader hot-reload in debug mode
#if defined(_DEBUG) && defined(_WIN32)
			// Store last write time
			m_lwt = in.GetLastWriteTime();
			SetupChangeHandler();
#endif
			return true;
		}

		bool UpdateHotReload()
		{
#ifdef _WIN32
			if (m_changeNotification != INVALID_HANDLE_VALUE)
			{
				if (WaitForSingleObject(m_changeNotification, 0) == WAIT_OBJECT_0)
				{
					const uint64 newLwt = File::GetLastWriteTime(m_sourcePath);
					if (newLwt != -1 && newLwt > m_lwt)
					{
						uint32 newProgram = 0;
						if (LoadProgram(newProgram))
						{
							// Successfully reloaded
							m_prog = newProgram;
							return true;
						}
					}

					// Watch for new change
					SetupChangeHandler();
				}
			}
#endif
			return false;
		}

		bool Init(ShaderType type, const String& name)
		{
			m_sourcePath = Path::Normalize(name);
			m_type = type;
			return LoadProgram(m_prog);
		}

		virtual void Bind()
		{
			if (m_gl->m_activeShaders[(size_t)m_type] != this)
			{
				glUseProgramStages(m_gl->m_mainProgramPipeline, shaderStageMap[(size_t)m_type], m_prog);
				m_gl->m_activeShaders[(size_t)m_type] = this;
			}
		}

		virtual bool IsBound() const
		{
			return m_gl->m_activeShaders[(size_t)m_type] == this;
		}

		virtual uint32 GetLocation(const String& name) const
		{
			return glGetUniformLocation(m_prog, name.c_str());
		}

		virtual void BindUniform(uint32 loc, const Transform& mat)
		{
			glProgramUniformMatrix4fv(m_prog, loc, 1, false, mat.mat);
		}

		virtual void BindUniformVec2(uint32 loc, const Vector2& v)
		{
			glProgramUniform2fv(m_prog, loc, 1, &v.x);
		}

		virtual void BindUniformVec3(uint32 loc, const Vector3& v)
		{
			glProgramUniform3fv(m_prog, loc, 1, &v.x);
		}

		virtual void BindUniformVec4(uint32 loc, const Vector4& v)
		{
			glProgramUniform4fv(m_prog, loc, 1, &v.x);
		}

		virtual void BindUniform(uint32 loc, int i)
		{
			glProgramUniform1i(m_prog, loc, i);
		}

		virtual void BindUniform(uint32 loc, float i)
		{
			glProgramUniform1f(m_prog, loc, i);
		}

		virtual void BindUniformArray(uint32 loc, const Transform* mat, size_t count)
		{
			glProgramUniformMatrix4fv(m_prog, loc, (int)count, false, (float*)mat);
		}

		virtual void BindUniformArray(uint32 loc, const Vector2* v2, size_t count)
		{
			glProgramUniform2fv(m_prog, loc, (int)count, (float*)v2);
		}

		virtual void BindUniformArray(uint32 loc, const Vector3* v3, size_t count)
		{
			glProgramUniform3fv(m_prog, loc, (int)count, (float*)v3);
		}

		virtual void BindUniformArray(uint32 loc, const Vector4* v4, size_t count)
		{
			glProgramUniform4fv(m_prog, loc, (int)count, (float*)v4);
		}

		virtual void BindUniformArray(uint32 loc, const float* i, size_t count)
		{
			glProgramUniform1fv(m_prog, loc, (int)count, i);
		}

		virtual void BindUniformArray(uint32 loc, const int* i, size_t count)
		{
			glProgramUniform1iv(m_prog, loc, (int)count, i);
		}

		virtual uint32 Handle() override
		{
			return m_prog;
		}

		String GetOriginalName() const
		{
			return m_sourcePath;
		}
	};

	Shader ShaderRes::Create(shared_ptr<OpenGL> gl, ShaderType type, const String& assetPath)
	{
		const auto pImpl = make_shared<Shader_Impl>(gl);
		if (!pImpl->Init(type, assetPath))
			return Shader();
		else
			return GetResourceManager<ResourceType::Shader>().Register(pImpl);
	}

	void ShaderRes::Unbind(shared_ptr<OpenGL> gl, ShaderType type)
	{
		if (gl->m_activeShaders[(size_t)type] != 0)
		{
			glUseProgramStages(gl->m_mainProgramPipeline, shaderStageMap[(size_t)type], 0);
			gl->m_activeShaders[(size_t)type] = 0;
		}
	}
}
