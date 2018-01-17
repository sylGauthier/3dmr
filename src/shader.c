#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glew.h>

#define MAX_INCLUDE_DEPTH 16

static int check_compilation(GLuint shader, const char* path) {
    GLint error, size;
    char* errorString;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
    if (error != GL_TRUE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        if ((errorString = malloc(size))) {
            glGetShaderInfoLog(shader, size, &size, errorString);
            errorString[size] = '\0';
            fprintf(stderr, "Error: failed to compile '%s':\n%s\n", path ? path : "<in memory shader>", errorString);
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
            glGetProgramInfoLog(prog, size, &size, errorString);
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

static GLuint compile_string(const char* code, GLint size, GLenum type, const char* filepathHint) {
    GLuint shader, ret = 0;

    if ((shader = glCreateShader(type))) {
        glShaderSource(shader, 1, (const char**)&code, &size);
        glCompileShader(shader);
        if (check_compilation(shader, filepathHint)) {
            ret = shader;
        } else {
            glDeleteShader(shader);
        }
    } else {
        fprintf(stderr, "Error: failed to create shader\n");
    }

    return ret;
}

static GLuint compile(const char* path, GLenum type) {
    char buffer[2048];
    GLuint ret = 0;
    GLint size = 0;
    char *code = NULL, *tmp, *start, *end, *currentPath[MAX_INCLUDE_DEPTH];
    unsigned int directorySize, oldSize = 0, newSize, lineSize, ifLevel = 0;
    FILE* source[MAX_INCLUDE_DEPTH];
    int i, currentSource = 0, ok = 1;

    if (!(source[0] = fopen(path, "r"))) {
        fprintf(stderr, "Error: failed to open '%s'\n", path);
        return 0;
    }
    currentPath[0] = (char*)path;

    do {
        while (fgets(buffer, sizeof(buffer), source[currentSource])) {
            if (!strncmp(buffer, "#include", 8) && isspace(buffer[8])) {
                if (ifLevel) continue;
                ok = 0;
                for (start = buffer + 9; *start && *start != '\"'; start++);
                if (*start != '\"' || !(end = strchr(++start, '\"'))) {
                    fprintf(stderr, "Error: failed to preprocess shader, #include syntax error\n\tin %s\n", currentPath[currentSource]);
                    for (i = currentSource - 1; i >= 0; i--) {
                        fprintf(stderr, "\t included from %s\n", currentPath[i]);
                    }
                } else if (currentSource >= (sizeof(source) / sizeof(*source)) - 1) {
                    fprintf(stderr, "Error: failed to preprocess shader, max #include depth reached\n\tin %s\n", currentPath[currentSource]);
                    for (i = currentSource - 1; i >= 0; i--) {
                        fprintf(stderr, "\t included from %s\n", currentPath[i]);
                    }
                } else {
                    *end = 0;
                    directorySize = ((tmp = strrchr(currentPath[currentSource], '/'))) ? (tmp - currentPath[currentSource] + 1) : 0;
                    if ((currentPath[currentSource + 1] = malloc(directorySize + end - start + 1))) {
                        currentSource++;
                        memcpy(currentPath[currentSource], currentPath[currentSource - 1], directorySize);
                        memcpy(currentPath[currentSource] + directorySize, start, end - start + 1);
                        if ((source[currentSource] = fopen(currentPath[currentSource], "r"))) {
                            ok = 1;
                        } else {
                            fprintf(stderr, "Error: failed to open included shader '%s'\n\tin %s\n", currentPath[currentSource], currentPath[currentSource - 1]);
                            currentSource--;
                            for (i = currentSource - 1; i >= 0; i--) {
                                fprintf(stderr, "\t included from %s\n", currentPath[i]);
                            }
                        }
                    } else {
                        fprintf(stderr, "Error: memory allocation failed\n");
                    }
                }
                break;
            } else {
                if ((!strncmp(buffer, "#if", 3) && isspace(buffer[3]))
                 || (!strncmp(buffer, "#ifdef", 6) && isspace(buffer[6]))
                 || (!strncmp(buffer, "#ifndef", 7) && isspace(buffer[7]))) {
                    ifLevel++;
                } else if (ifLevel && !strncmp(buffer, "#endif", 6) && isspace(buffer[6])) {
                    ifLevel--;
                }
                lineSize = strlen(buffer);
                if (oldSize < (newSize = ((size + lineSize + 1023) / 1024) * 1024)) {
                    if ((tmp = realloc(code, newSize))) {
                        code = tmp;
                        oldSize = newSize;
                    } else {
                        fprintf(stderr, "Error: memory allocation failed\n");
                        ok = 0;
                        break;
                    }
                }
                memcpy(code + size, buffer, lineSize);
                size += lineSize;
            }
        }
        if (feof(source[currentSource])) {
            fclose(source[currentSource]);
            if (currentSource) free(currentPath[currentSource]);
            currentSource--;
        }
    } while (ok && currentSource >= 0);

    while (currentSource >= 0) {
        fclose(source[currentSource]);
        if (currentSource) free(currentPath[currentSource]);
        currentSource--;
    }

    if (ok) {
        /*printf("Compiling %s\n", path);
        fwrite(code, 1, size, stdout);*/
        ret = compile_string(code, size, type, path);
    }
    free(code);

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
