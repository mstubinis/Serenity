#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/opengl/Extensions.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;


std::array<bool, OpenGLExtensions::_TOTAL> OPENGL_EXTENSIONS {

};

void OpenGLExtensions::INIT() noexcept {
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_texture_filter_anisotropic]  = checkOpenGLExtension("GL_EXT_texture_filter_anisotropic");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_texture_filter_anisotropic]  = checkOpenGLExtension("GL_ARB_texture_filter_anisotropic");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_draw_instanced]              = checkOpenGLExtension("GL_EXT_draw_instanced");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_draw_instanced]              = checkOpenGLExtension("GL_ARB_draw_instanced");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_separate_shader_objects]     = checkOpenGLExtension("GL_EXT_separate_shader_objects");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_separate_shader_objects]     = checkOpenGLExtension("GL_ARB_separate_shader_objects");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_explicit_attrib_location]    = checkOpenGLExtension("GL_EXT_explicit_attrib_location");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_explicit_attrib_location]    = checkOpenGLExtension("GL_ARB_explicit_attrib_location");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_geometry_shader4]            = checkOpenGLExtension("GL_EXT_geometry_shader4");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_geometry_shader4]            = checkOpenGLExtension("GL_ARB_geometry_shader4");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_compute_shader]              = checkOpenGLExtension("GL_EXT_compute_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_compute_shader]              = checkOpenGLExtension("GL_ARB_compute_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_tessellation_shader]         = checkOpenGLExtension("GL_EXT_tessellation_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_tessellation_shader]         = checkOpenGLExtension("GL_ARB_tessellation_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_bindless_texture]            = checkOpenGLExtension("GL_ARB_bindless_texture");
    OPENGL_EXTENSIONS[OpenGLExtensions::NV_bindless_texture]             = checkOpenGLExtension("GL_NV_bindless_texture");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_gpu_shader_int64]            = checkOpenGLExtension("GL_ARB_bindless_texture");
    OPENGL_EXTENSIONS[OpenGLExtensions::NV_gpu_shader5]                  = checkOpenGLExtension("GL_NV_bindless_texture");

    #if !defined(ENGINE_PRODUCTION) && defined(ENGINE_PRINT_OPENGL_EXTENSIONS)
        printAllAvailableExtensions();
        for (size_t i = 0; i < OPENGL_EXTENSIONS.size(); ++i) {
            std::cout << OPENGL_EXTENSIONS[i] << "\n";
        }
    #endif
}

bool OpenGLExtensions::checkOpenGLExtension(const char* e) noexcept {
    if (glewIsExtensionSupported(e) != 0) {
        return true;
    }
    return (0 != glewIsSupported(e)); 
}
void OpenGLExtensions::printAllAvailableExtensions() noexcept {
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    std::string output = "";
    for (GLint i = 0; i < n; ++i) {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        output += std::to_string(i) + ": " + extension + "\n";
    }
    std::cout << output << "\n";
}

bool OpenGLExtensions::supported(OpenGLExtensions::Extension extension) noexcept {
    return OPENGL_EXTENSIONS[extension];
}
bool OpenGLExtensions::isBindlessTexturesSupported() noexcept {
    return OPENGL_EXTENSIONS[OpenGLExtensions::ARB_bindless_texture] || OPENGL_EXTENSIONS[OpenGLExtensions::NV_bindless_texture];
}