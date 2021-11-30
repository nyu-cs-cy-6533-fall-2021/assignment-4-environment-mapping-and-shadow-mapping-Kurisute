#ifndef SHADOW_MAP_FBO_H
#define SHADOW_MAP_FBO_H

#include <string>
#include <vector>
#include <glm/glm.hpp>  // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4


#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#  undef DrawText
#endif

#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ /* Prevent inclusion of the old gl.h */
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/gl.h>
#endif

class ShadowMapFBO
{
public:
    typedef unsigned int GLuint;
    typedef int GLint;
    GLuint id;

    ShadowMapFBO() : id(0) {}

    void init();

    void bind();

    void free();
};

#endif