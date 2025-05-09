#include "gl4_util.h"

#include "gl4_local.h"
#include <string.h>

GLuint check_compile_errors(GLuint shader, const char* type)
{
    int success;
    char infoLog[1024];

    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            gEngfuncs.Con_Printf(S_ERROR "Shader compilation error: %s\n%s\n", type,
                infoLog);
            return GL_FALSE;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            gEngfuncs.Con_Printf(S_ERROR "Shader linking error: %s\n%s\n", type,
                infoLog);
            return GL_FALSE;
        }
    }

    return GL_TRUE;
}

GLuint compile_shader(const char* vs_shader_source,
    const char* fs_shader_source)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs_shader_source, NULL);
    glCompileShader(vertexShader);
    if (check_compile_errors(vertexShader, "VERTEX") == GL_FALSE) {
        glDeleteShader(vertexShader);
        return GL_FALSE;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs_shader_source, NULL);
    glCompileShader(fragmentShader);
    if (check_compile_errors(fragmentShader, "FRAGMENT") == GL_FALSE) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return GL_FALSE;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    if (check_compile_errors(shaderProgram, "PROGRAM") == GL_FALSE) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return GL_FALSE;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    gEngfuncs.Con_Printf("Shader program compiled successfully\n\tID = %d\n",
        shaderProgram);
    return shaderProgram;
}
