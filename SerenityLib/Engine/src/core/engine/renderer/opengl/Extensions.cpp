#include <core/engine/renderer/opengl/Extensions.h>
#include <iostream>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

vector<bool> OpenGLExtensions::OPENGL_EXTENSIONS = []() {
    vector<bool> v;
    v.resize(OpenGLExtensions::_TOTAL, false);
    return v;
}();

OpenGLExtensions::OpenGLExtensions() {

}
OpenGLExtensions::~OpenGLExtensions() {

}
void OpenGLExtensions::INIT() {
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_Ansiotropic_Filtering]    = checkOpenGLExtension("GL_EXT_texture_filter_anisotropic");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_Ansiotropic_Filtering]    = checkOpenGLExtension("GL_ARB_texture_filter_anisotropic");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_draw_instanced]           = checkOpenGLExtension("GL_EXT_draw_instanced");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_draw_instanced]           = checkOpenGLExtension("GL_ARB_draw_instanced");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_separate_shader_objects]  = checkOpenGLExtension("GL_EXT_separate_shader_objects");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_separate_shader_objects]  = checkOpenGLExtension("GL_ARB_separate_shader_objects");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_explicit_attrib_location] = checkOpenGLExtension("GL_EXT_explicit_attrib_location");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_explicit_attrib_location] = checkOpenGLExtension("GL_ARB_explicit_attrib_location");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_geometry_shader_4]        = checkOpenGLExtension("GL_EXT_geometry_shader4");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_geometry_shader_4]        = checkOpenGLExtension("GL_ARB_geometry_shader4");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_compute_shader]           = checkOpenGLExtension("GL_EXT_compute_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_compute_shader]           = checkOpenGLExtension("GL_ARB_compute_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::EXT_tessellation_shader]      = checkOpenGLExtension("GL_EXT_tessellation_shader");
    OPENGL_EXTENSIONS[OpenGLExtensions::ARB_tessellation_shader]      = checkOpenGLExtension("GL_ARB_tessellation_shader");
}

const bool OpenGLExtensions::checkOpenGLExtension(const char* e) {
    if (glewIsExtensionSupported(e) != 0) 
        return true; 
    return 0 != glewIsSupported(e); 
}
void OpenGLExtensions::printAllAvailableExtensions() {
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; ++i) {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        cout << "Ext " << i << ": " << extension << endl;
    }
}

const bool OpenGLExtensions::supported(const OpenGLExtensions::Extension& extension) {
    return OpenGLExtensions::OPENGL_EXTENSIONS[extension];
}