#include "stdafx.h"
#include "Shader.hpp"
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

	/**
	 * \throws std::runtime_error if failed to load shader program
	 */
	ShaderRes::ShaderRes(shared_ptr<OpenGL> gl, ShaderType type, const String& assetPath)
		: m_gl(std::move(gl))
	{
		m_sourcePath = Path::Normalize(assetPath);
		m_type = type;
		if (!LoadProgram(m_prog))
		{
			Logf("Failed to load shader program %s", Logger::Error, assetPath);
			throw std::runtime_error("Failed to load shader program");
		}
	}

	ShaderRes::~ShaderRes()
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

	void ShaderRes::Unbind(shared_ptr<OpenGL> gl, ShaderType type)
	{
		if (gl->m_activeShaders[(size_t)type] != 0)
		{
			glUseProgramStages(gl->m_mainProgramPipeline, shaderStageMap[(size_t)type], 0);
			gl->m_activeShaders[(size_t)type] = 0;
		}
	}

	bool ShaderRes::UpdateHotReload()
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
				setup_change_handler();
			}
		}
#endif
		return false;
	}

	void ShaderRes::Bind()
	{
		if (m_gl->m_activeShaders[static_cast<size_t>(m_type)] != this)
		{
			glUseProgramStages(m_gl->m_mainProgramPipeline, shaderStageMap[static_cast<size_t>(m_type)], m_prog);
			m_gl->m_activeShaders[static_cast<size_t>(m_type)] = this;
		}
	}

	bool ShaderRes::IsBound() const
	{
		return m_gl->m_activeShaders[static_cast<size_t>(m_type)] == this;
	}

	uint32 ShaderRes::GetLocation(const String& name) const
	{
		return glGetUniformLocation(m_prog, name.c_str());
	}

	uint32 ShaderRes::Handle() const
	{
		return m_prog;
	}

	String ShaderRes::GetOriginalName() const
	{
		return m_sourcePath;
	}

	bool ShaderRes::LoadProgram(uint32& programOut)
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
		setup_change_handler();
#endif
		return true;
	}

	void ShaderRes::setup_change_handler()
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

	void ShaderRes::BindUniform(uint32 loc, const Transform& mat) const
	{
		glProgramUniformMatrix4fv(m_prog, loc, 1, false, mat.mat);
	}

	void ShaderRes::BindUniformVec2(uint32 loc, const Vector2& v) const
	{
		glProgramUniform2fv(m_prog, loc, 1, &v.x);
	}

	void ShaderRes::BindUniformVec3(uint32 loc, const Vector3& v) const
	{
		glProgramUniform3fv(m_prog, loc, 1, &v.x);
	}

	void ShaderRes::BindUniformVec4(uint32 loc, const Vector4& v) const
	{
		glProgramUniform4fv(m_prog, loc, 1, &v.x);
	}

	void ShaderRes::BindUniform(uint32 loc, int i) const
	{
		glProgramUniform1i(m_prog, loc, i);
	}

	void ShaderRes::BindUniform(uint32 loc, float i) const
	{
		glProgramUniform1f(m_prog, loc, i);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const Transform* mat, size_t count) const
	{
		glProgramUniformMatrix4fv(m_prog, loc, static_cast<int>(count), false, (float*)mat);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const Vector2* v2, size_t count) const
	{
		glProgramUniform2fv(m_prog, loc, static_cast<int>(count), (float*)v2);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const Vector3* v3, size_t count) const
	{
		glProgramUniform3fv(m_prog, loc, static_cast<int>(count), (float*)v3);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const Vector4* v4, size_t count) const
	{
		glProgramUniform4fv(m_prog, loc, static_cast<int>(count), (float*)v4);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const float* i, size_t count) const
	{
		glProgramUniform1fv(m_prog, loc, static_cast<int>(count), i);
	}

	void ShaderRes::BindUniformArray(uint32 loc, const int* i, size_t count) const
	{
		glProgramUniform1iv(m_prog, loc, static_cast<int>(count), i);
	}
}