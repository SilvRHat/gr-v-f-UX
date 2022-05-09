// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// GRAPHICS
#ifndef MV_RE_GRAPHICS_H
#define MV_RE_GRAPHICS_H


// DEPENDENCIES
#ifdef USEGLEW
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/gl.h>
#endif


#endif