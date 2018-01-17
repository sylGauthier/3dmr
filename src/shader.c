#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

static int check_compilation(GLuint shader, const char* path) {
    GLint error, size;
    char* errorString;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
    if (error != GL_TRUE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        if ((errorString = malloc(size))) {
            glGetShaderInfoLog(shader, size, &size, errorString);
            errorString[size] = '\0';
            fprintf(stderr, "Error: failed to compile '%s':\n%s\n", path, errorString);
            free(errorString);
        } else {
            fprintf(stderr, "Error: failed to compile '%s' and failed to retrieve the compilation error\n", path);
        }
        return 0;
    }

    return 1;
}

static int check_link(GLuint prog) {
    GLint error, size;
    char* errorString;

    glGetProgramiv(prog, GL_LINK_STATUS, &error);
    if (error != GL_TRUE) {
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
        if ((errorString = malloc(size))) {
            glGetShaderInfoLog(prog, size, &size, errorString);
            errorString[size] = '\0';
            fprintf(stderr, "Error: failed to link shaders:\n%s\n", errorString);
            free(errorString);
        } else {
            fprintf(stderr, "Error: failed to link shaders and to retrieve the error\n");
        }
        return 0;
    }

    return 1;
}


static GLuint compile(const char* path, GLenum type) {
    GLuint shader, ret = 0;
    GLint size;
    char* code;
    FILE* source;

    if ((shader = glCreateShader(type))) {
        if ((source = fopen(path, "rb")) && !fseek(source, 0, SEEK_END)) {
            if ((code = malloc(size = ftell(source)))) {
                if (!fseek(source, 0, SEEK_SET) && fread(code, 1, size, source) == size && !ferror(source)) {
                    glShaderSource(shader, 1, (const char**)&code, &size);
                    glCompileShader(shader);
                    if (check_compilation(shader, path)) {
                        ret = shader;
                    } else {
                        glDeleteShader(shader);
                    }
                } else {
                    fprintf(stderr, "Error: failed to read '%s'\n", path);
                }
                free(code);
            } else {
                fprintf(stderr, "Error: failed to allocate memory for '%s'\n", path);
            }
            if (source) {
                fclose(source);
            }
        } else {
            fprintf(stderr, "Error: failed to open '%s' for reading\n", path);
        }
    } else {
        fprintf(stderr, "Error: failed to create shader\n");
    }

    return ret;
}

GLuint shader_compile(const char* vertexShaderPath, const char* fragmentShaderPath) {
    GLuint prog = 0, vertexShader, fragmentShader;

    if ((vertexShader = compile(vertexShaderPath, GL_VERTEX_SHADER))) {
        if ((fragmentShader = compile(fragmentShaderPath, GL_FRAGMENT_SHADER))) {
            prog = glCreateProgram();
            glAttachShader(prog, vertexShader);
            glAttachShader(prog, fragmentShader);
            glBindAttribLocation(prog, 0, "in_Vertex");
            glBindAttribLocation(prog, 1, "in_Normal");
            glBindAttribLocation(prog, 2, "in_TexCoord");
            glLinkProgram(prog);
            if (check_link(prog)) {
                glDetachShader(prog, vertexShader);
                glDetachShader(prog, fragmentShader);
            } else {
                fprintf(stderr, "The shaders were '%s' and '%s'\n", vertexShaderPath, fragmentShaderPath);
                glDeleteProgram(prog);
                prog = 0;
            }
            glDeleteShader(fragmentShader);
        }
        glDeleteShader(vertexShader);
    }

    return prog;
}
