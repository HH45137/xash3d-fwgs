#ifndef GL4_UTIL_H
#define GL4_UTIL_H

#include "glad.h"

GLuint check_compile_errors(GLuint shader, const char *type);

GLuint compile_shader(const char *vs_shader_source,
                      const char *fs_shader_source);

void check_opengl_error(const char *context);

#endif // GL4_UTIL_H
