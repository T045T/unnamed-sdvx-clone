#pragma once
#include <Graphics/GL.hpp>
#include <Graphics/Window.hpp>
#include <Graphics/Framebuffer.hpp>

namespace Graphics
{
#ifdef _WIN32
	void __stdcall GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
								const GLchar* message, const void* userParam);
#else
	void GLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

	/*
		OpenGL context wrapper with common functionality
	*/
	class OpenGL
	{
	public:
		OpenGL();
		~OpenGL();
		bool Init(Window& window);
		void UnbindFramebuffer();

		Recti GetViewport() const;
		void SetViewport(Vector2i size);
		void SetViewport(Recti vp);

		// Check if the calling thread is the thread that runs this OpenGL context
		bool IsOpenGLThread() const;

		FramebufferRes* get_framebuffer() const;
		void set_framebuffer(FramebufferRes* buffer);

		virtual void SwapBuffers();

	private:
		class ShaderRes* m_activeShaders[3] = {0};
		uint32 m_mainProgramPipeline;
		class OpenGL_Impl* m_impl;
		Window* m_window;
		class FramebufferRes* m_boundFramebuffer;

		friend class ShaderRes;
		friend class TextureRes;
		friend class MeshRes;
		friend class ShaderRes;
		friend class RenderQueue;
	};
}
