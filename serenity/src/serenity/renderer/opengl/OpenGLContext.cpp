#include <serenity/renderer/opengl/OpenGLContext.h>
#include <serenity/system/Macros.h>
#include <serenity/system/window/Window.h>
#include <SFML/Window/ContextSettings.hpp>
#include <array>

#include <GL/glew.h>

#ifdef _WIN32
	#include <Windows.h>
	#include <GL/gl.h>
	//#include <GL/glext.h>
	#include <GL/wglext.h>
#endif
#ifdef linux
	#include <GL/glx.h>
	#include <X11/Xlib.h>
    using (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*) = GLXContext;
#endif

namespace {

	std::vector<bool> OPENGL_EXTENSIONS(Engine::priv::OpenGLContext::Extensions::_TOTAL, false);

#ifdef _WIN32
	PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB = nullptr;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
	PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT = nullptr;

	std::string win32_get_last_error() {
		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;
		DWORD errorMessageID = GetLastError();
		std::string message;
		if (errorMessageID != 0) {
			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
			message = std::string(messageBuffer, size);
			LocalFree(messageBuffer);
		}
		return message;
	}

	auto get_extension(auto*& ptr, std::string&& extName) {
		ptr = reinterpret_cast<std::remove_reference_t<decltype(ptr)>>(wglGetProcAddress(extName.c_str()));
		if (ptr == nullptr) {
			ENGINE_PRODUCTION_LOG("wglGetProcAddress(" + extName + ") - failed.");
			return false;
		}
		return ptr != nullptr;
	}
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	bool register_window_class() {
		WNDCLASSEX wcex = { 0 };
		ZeroMemory(&wcex, sizeof(wcex));
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hIcon = LoadIcon(0, IDI_WINLOGO);
		wcex.lpszMenuName = 0;
		wcex.hbrBackground = 0;
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = WndProc;
		wcex.hInstance = GetModuleHandle(nullptr);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = L"Core";
		if (!RegisterClassEx(&wcex)) {
			ENGINE_PRODUCTION_LOG("CreateWindow() - failed.");
			return false;
		}
		return true;
	}
	bool create_fake_window_and_device_context(HWND& fakeWindow, HDC& fakeDeviceContext) {
		fakeWindow = CreateWindowEx(0, L"Core", L"FakeWindow", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(nullptr), NULL);
		if (!fakeWindow) {
			ENGINE_PRODUCTION_LOG("CreateWindowEx() - failed.");
			return false;
		}
		fakeDeviceContext = GetDC(fakeWindow);
		if (!fakeDeviceContext) {
			ENGINE_PRODUCTION_LOG("GetDC(fakeWindow) - failed.");
			return false;
		}
		return true;
	}
	bool create_fake_pixel_format(HDC fakeDeviceContext) {
		PIXELFORMATDESCRIPTOR fakePFD;
		ZeroMemory(&fakePFD, sizeof(fakePFD));
		fakePFD.nSize = sizeof(fakePFD);
		fakePFD.nVersion = 1;
		fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		fakePFD.iPixelType = PFD_TYPE_RGBA;
		const auto bitsPerPixel = sf::VideoMode::getDesktopMode().bitsPerPixel;
		fakePFD.cColorBits = static_cast<BYTE>(sf::VideoMode::getDesktopMode().bitsPerPixel);
		fakePFD.cDepthBits = static_cast<BYTE>(24);
		fakePFD.cStencilBits = static_cast<BYTE>(8);
		fakePFD.cAlphaBits = bitsPerPixel == 32 ? 8 : 0;

		int fakePFDID = ChoosePixelFormat(fakeDeviceContext, &fakePFD);
		if (fakePFDID == 0) {
			ENGINE_PRODUCTION_LOG("ChoosePixelFormat(fakeDeviceContext, &fakePFD) - failed.");
			return false;
		}
		if (SetPixelFormat(fakeDeviceContext, fakePFDID, &fakePFD) == false) {
			ENGINE_PRODUCTION_LOG("SetPixelFormat(fakeDeviceContext, fakePFDID, &fakePFD) - failed.");
			return false;
		}
		return true;
	}
	bool create_and_make_current_fake_rendering_context(HDC fakeDeviceContext, HGLRC& fakeRenderingContext) {
		fakeRenderingContext = wglCreateContext(fakeDeviceContext);
		if (fakeRenderingContext == 0) {
			ENGINE_PRODUCTION_LOG("wglCreateContext(fakeDeviceContext) - failed.");
			return false;
		}
		if (wglMakeCurrent(fakeDeviceContext, fakeRenderingContext) == false) {
			ENGINE_PRODUCTION_LOG("wglMakeCurrent(fakeDeviceContext, fakeRenderingContext) - failed.");
			return false;
		}
		return true;
	}
	void delete_fake_window_and_context(HGLRC& fakeRenderingContext, HDC& fakeDeviceContext, HWND& fakeWindow) {
		if (fakeRenderingContext) {
			wglDeleteContext(fakeRenderingContext);
		}
		if (fakeDeviceContext && fakeWindow) {
			ReleaseDC(fakeWindow, fakeDeviceContext);
			DestroyWindow(fakeWindow);
		}
	}

	bool choose_pixel_format(HDC& deviceContext, std::array<int, 512>& pixelFormatIDs) {
		const int pixelAttribs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 4,
			0
		};

		UINT numFormats = 0;
		bool status = wglChoosePixelFormatARB(deviceContext, pixelAttribs, nullptr, pixelFormatIDs.size(), pixelFormatIDs.data(), &numFormats);

		if (status == false || numFormats == 0) {
			ENGINE_PRODUCTION_LOG("wglChoosePixelFormatARB(deviceContext, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats) - failed.");
			return false;
		}
		return true;
	}
	bool set_pixel_format(HDC& deviceContext, int pixelFormatID) {
		PIXELFORMATDESCRIPTOR PFD;
		DescribePixelFormat(deviceContext, pixelFormatID, sizeof(PFD), &PFD);
		if (!SetPixelFormat(deviceContext, pixelFormatID, &PFD)) {
			return false;
		}
		return true;
	}
	bool create_rendering_context(HDC& deviceContext, HGLRC& renderingContext) {
		if (renderingContext) {
			return true;
		}
		std::vector<int> contextAttribs;
		contextAttribs.reserve(10);
		contextAttribs.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
		contextAttribs.push_back(4);
		contextAttribs.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
		contextAttribs.push_back(6);

		contextAttribs.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
		contextAttribs.push_back(WGL_CONTEXT_CORE_PROFILE_BIT_ARB); //or WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
		contextAttribs.push_back(WGL_CONTEXT_FLAGS_ARB);

#ifdef _DEBUG
		contextAttribs.push_back(WGL_CONTEXT_DEBUG_BIT_ARB);
#else
		contextAttribs.push_back(0);
#endif

		// Append the terminating 0's
		contextAttribs.push_back(0);
		contextAttribs.push_back(0);

		int& chosenMajor = contextAttribs[1];
		int& chosenMinor = contextAttribs[3];

		const int majorMustBeAtLeast = 3;
		const int minorMustBeAtLeast = 3;
		const int mustBeAtLeastRepresentation = minorMustBeAtLeast + (majorMustBeAtLeast * 10);

		//chose the highest version capable while being at least the required version
		while (renderingContext == NULL && (chosenMinor + (chosenMajor * 10) >= mustBeAtLeastRepresentation)) {
			renderingContext = wglCreateContextAttribsARB(deviceContext, 0, contextAttribs.data());
			--chosenMinor;
			if (chosenMinor <= -1) {
				chosenMinor = 9;
				--chosenMajor;
			}
		}
		if (renderingContext == NULL) {
			ENGINE_PRODUCTION_LOG("wglCreateContextAttribsARB(deviceContext, 0, contextAttribs.data()) - failed.");
			return false;
		}
		return true;
	}
#elif defined(linux)

#endif
}

namespace Engine::priv {
	bool OpenGLContext::checkOpenGLExtension(const char* e) noexcept {
		return (glewIsExtensionSupported(e) != 0) ? true : (0 != glewIsSupported(e));
	}
	bool OpenGLContext::supported(Extensions::Type extension) noexcept {
		return OPENGL_EXTENSIONS[extension];
	}
	bool OpenGLContext::isBindlessTexturesSupported() noexcept {
		return OPENGL_EXTENSIONS[Extensions::ARB_bindless_texture] || OPENGL_EXTENSIONS[Extensions::NV_bindless_texture];
	}
	void OpenGLContext::printAllAvailableExtensions() noexcept {
		GLint n = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		std::string output;
		output.reserve(n * 4);
		for (GLint i = 0; i < n; ++i) {
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
			output += std::to_string(i) + ": " + extension + '\n';
		}
		ENGINE_PRODUCTION_LOG(output)
	}


#if defined(_WIN32)
	struct OpenGLContext::impl {
		HDC    m_DeviceContext = nullptr; // this is specific to each window

		HGLRC  m_RenderContext = nullptr; // in theory, this can be shared between multiple windows
	};

	OpenGLContext::OpenGLContext()
		: m_i {std::make_unique<impl>()}
	{
	}
	bool OpenGLContext::init(Window& window) {
		HWND fakeWindow            = nullptr;
		HDC fakeDeviceContext      = nullptr;
		HGLRC fakeRenderingContext = nullptr;
		if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT) {
			if (!register_window_class()) { return false; }
			if (!create_fake_window_and_device_context(fakeWindow, fakeDeviceContext)) { return false; }
			if (!create_fake_pixel_format(fakeDeviceContext)) { return false; }
			if (!create_and_make_current_fake_rendering_context(fakeDeviceContext, fakeRenderingContext)) { return false; }

			if (!get_extension(wglChoosePixelFormatARB, "wglChoosePixelFormatARB")) { return false; }
			if (!get_extension(wglCreateContextAttribsARB, "wglCreateContextAttribsARB")) { return false; }
			if (!get_extension(wglSwapIntervalEXT, "wglSwapIntervalEXT")) { return false; }
		}

		auto windowOSHandle = window.getSystemHandle();
		int releasedOldDC = 0;
		if (m_i->m_DeviceContext) {
			releasedOldDC = ReleaseDC(windowOSHandle, m_i->m_DeviceContext);
		}
		m_i->m_DeviceContext = GetDC(windowOSHandle);

		std::array<int, 512> pixelFormatIDs;
		if (!choose_pixel_format(m_i->m_DeviceContext, pixelFormatIDs)) {
			return false;
		}
		if (!set_pixel_format(m_i->m_DeviceContext, pixelFormatIDs[0])) {
			return false;
		}
		if (!create_rendering_context(m_i->m_DeviceContext, m_i->m_RenderContext)) {
			return false;
		}

		wglMakeCurrent(NULL, NULL);

		delete_fake_window_and_context(fakeRenderingContext, fakeDeviceContext, fakeWindow);

		if (!wglMakeCurrent(m_i->m_DeviceContext, m_i->m_RenderContext)) {
			ENGINE_PRODUCTION_LOG("wglMakeCurrent() - failed.");
			return false;
		}


		glewExperimental = GL_TRUE;
		glewInit();
		glGetError();//stupid glew always inits an error. nothing we can do about it.

		OPENGL_EXTENSIONS[Extensions::EXT_texture_filter_anisotropic] = OpenGLContext::checkOpenGLExtension("GL_EXT_texture_filter_anisotropic");
		OPENGL_EXTENSIONS[Extensions::ARB_texture_filter_anisotropic] = OpenGLContext::checkOpenGLExtension("GL_ARB_texture_filter_anisotropic");
		OPENGL_EXTENSIONS[Extensions::EXT_draw_instanced] = OpenGLContext::checkOpenGLExtension("GL_EXT_draw_instanced");
		OPENGL_EXTENSIONS[Extensions::ARB_draw_instanced] = OpenGLContext::checkOpenGLExtension("GL_ARB_draw_instanced");
		OPENGL_EXTENSIONS[Extensions::EXT_separate_shader_objects] = OpenGLContext::checkOpenGLExtension("GL_EXT_separate_shader_objects");
		OPENGL_EXTENSIONS[Extensions::ARB_separate_shader_objects] = OpenGLContext::checkOpenGLExtension("GL_ARB_separate_shader_objects");
		OPENGL_EXTENSIONS[Extensions::EXT_explicit_attrib_location] = OpenGLContext::checkOpenGLExtension("GL_EXT_explicit_attrib_location");
		OPENGL_EXTENSIONS[Extensions::ARB_explicit_attrib_location] = OpenGLContext::checkOpenGLExtension("GL_ARB_explicit_attrib_location");
		OPENGL_EXTENSIONS[Extensions::EXT_geometry_shader4] = OpenGLContext::checkOpenGLExtension("GL_EXT_geometry_shader4");
		OPENGL_EXTENSIONS[Extensions::ARB_geometry_shader4] = OpenGLContext::checkOpenGLExtension("GL_ARB_geometry_shader4");
		OPENGL_EXTENSIONS[Extensions::EXT_compute_shader] = OpenGLContext::checkOpenGLExtension("GL_EXT_compute_shader");
		OPENGL_EXTENSIONS[Extensions::ARB_compute_shader] = OpenGLContext::checkOpenGLExtension("GL_ARB_compute_shader");
		OPENGL_EXTENSIONS[Extensions::EXT_tessellation_shader] = OpenGLContext::checkOpenGLExtension("GL_EXT_tessellation_shader");
		OPENGL_EXTENSIONS[Extensions::ARB_tessellation_shader] = OpenGLContext::checkOpenGLExtension("GL_ARB_tessellation_shader");
		OPENGL_EXTENSIONS[Extensions::ARB_bindless_texture] = OpenGLContext::checkOpenGLExtension("GL_ARB_bindless_texture");
		OPENGL_EXTENSIONS[Extensions::NV_bindless_texture] = OpenGLContext::checkOpenGLExtension("GL_NV_bindless_texture");
		OPENGL_EXTENSIONS[Extensions::ARB_gpu_shader_int64] = OpenGLContext::checkOpenGLExtension("GL_ARB_bindless_texture");
		OPENGL_EXTENSIONS[Extensions::NV_gpu_shader5] = OpenGLContext::checkOpenGLExtension("GL_NV_bindless_texture");
		OPENGL_EXTENSIONS[Extensions::ARB_clip_control] = OpenGLContext::checkOpenGLExtension("GL_ARB_clip_control");
		OPENGL_EXTENSIONS[Extensions::ARB_seamless_cube_map] = OpenGLContext::checkOpenGLExtension("GL_ARB_seamless_cube_map");
		//ARB_seamless_cube_map

#if !defined(ENGINE_PRODUCTION) && defined(ENGINE_PRINT_OPENGL_EXTENSIONS)
		printAllAvailableExtensions();
		for (size_t i = 0; i < OPENGL_EXTENSIONS.size(); ++i) {
			ENGINE_PRODUCTION_LOG(OPENGL_EXTENSIONS[i])
		}
#endif
		ENGINE_PRODUCTION_LOG("OpenGL Context created - " + std::string{ reinterpret_cast<const char*>(glGetString(GL_VERSION)) });
		return true;
	}
	void OpenGLContext::destroy(Window& window) {
		wglMakeCurrent(NULL, NULL);
		if (m_i->m_RenderContext) {
			wglDeleteContext(m_i->m_RenderContext);
		}
		if (m_i->m_DeviceContext) {
			if (ReleaseDC(window.getSystemHandle(), m_i->m_DeviceContext)) {
				m_i->m_DeviceContext = nullptr;
			}
		}
	}
	bool OpenGLContext::detatch(Window& window) {
		if (!wglMakeCurrent(m_i->m_DeviceContext ? m_i->m_DeviceContext : NULL, NULL)) {
			return false;
		}
		if (m_i->m_DeviceContext) {
			if (ReleaseDC(window.getSystemHandle(), m_i->m_DeviceContext)) {
				m_i->m_DeviceContext = nullptr;
			}

		}
		return true;
	}
	void OpenGLContext::setFramerateLimit(int limit) {

	}
	bool OpenGLContext::setVerticalSyncEnabled(bool enabled) {
		if (wglSwapIntervalEXT) {
			if (wglSwapIntervalEXT(enabled ? 1 : 0) == TRUE) {
				return true;
			}
		} 
		return false;
	}
	bool OpenGLContext::display() {
		return SwapBuffers(m_i->m_DeviceContext);
	}
#elif defined(linux)
	struct OpenGLContext::impl {
		GLXFBConfig  m_GLXFBConfig;	 
		GLXContext   m_GLXContext;
		Window*      m_Window  = nullptr;
		Display*     m_Display = nullptr;
	};


	OpenGLContext::OpenGLContext()
		: m_i{ std::make_unique<impl>() }
	{
	}
	bool OpenGLContext::init(Window& window) {	
		m_i->m_Display = XOpenDisplay(NULL);
		m_i->m_Window = std::addressof(window);

		const int visual_attribs[] = {
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_DOUBLEBUFFER, true,
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			None
		};
		int num_fbc = 0;
		m_i->m_GLXFBConfig = glXChooseFBConfig(m_i->m_Display, DefaultScreen(m_i->m_Display), visual_attribs, &num_fbc);
		if (!m_i->m_GLXFBConfig) {
			ENGINE_PRODUCTION_LOG("glXChooseFBConfig() failed");
			return false;
		}

		/* Set desired minimum OpenGL version */
		const int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 2,
			None
		};
		/* Create modern OpenGL context */
		m_i->m_GLXContext = glXCreateContextAttribsARB(m_i->m_Display, m_i->m_GLXFBConfig[0], NULL, true, context_attribs);
		if (!m_i->m_GLXContext) {
			ENGINE_PRODUCTION_LOG("Failed to create OpenGL context");
			return false;
		}
		XMapWindow(m_i->m_Display, window.getSystemHandle()); //TODO: is this line needed?
		glXMakeCurrent(m_i->m_Display, window.getSystemHandle(), m_i->m_GLXContext);

		return true;
	}
	void OpenGLContext::destroy(Window& window) {
		if (glXGetCurrentContext() == m_i->m_GLXContext) {
			//glXMakeCurrent(m_i->m_Display, None, nullptr);
			glXMakeCurrent(m_i->m_Display, 0, nullptr);
		}
		glXDestroyContext(m_i->m_Display, m_i->m_GLXContext);
		CloseDisplay(m_i->m_Display);
	}

	bool OpenGLContext::detatch(Window& window) {
		if (glXGetCurrentContext() == m_i->m_GLXContext) {
			//glXMakeCurrent(m_i->m_Display, None, nullptr);
			glXMakeCurrent(m_i->m_Display, 0, nullptr);
			return true;
		}
		return false;
	}
	void OpenGLContext::setFramerateLimit(int limit) {

	}
	bool OpenGLContext::setVerticalSyncEnabled(bool enabled) {
		int result = 0;
		// Prioritize the EXT variant and fall back to MESA or SGI if needed
		// We use the direct pointer to the MESA entry point instead of the alias
		// because glx.h declares the entry point as an external function
		// which would require us to link in an additional library
		if (glXSwapIntervalEXT) {
			glXSwapIntervalEXT(m_i->m_Display, m_i->m_Window->getSystemHandle(), enabled ? 1 : 0);
		} else if (glXSwapIntervalMESA) {
			result = glXSwapIntervalMESA(enabled ? 1 : 0);
		} else if (glXSwapIntervalSGI) {
			result = glXSwapIntervalSGI(enabled ? 1 : 0);
		} else {
			ENGINE_PRODUCTION_LOG("Setting vertical sync not supported");
			return false;
		}
		if (result != 0) {
			ENGINE_PRODUCTION_LOG("Setting vertical sync failed");
			return false;
		}
		return true;
	}
	bool OpenGLContext::display() {
		glXSwapBuffers(m_i->m_Display, m_i->m_Window->getSystemHandle());
		return true;
	}
#endif

}