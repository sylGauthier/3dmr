#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <GL/glew.h>
#include <game/render/shader.h>

#define MAX_INCLUDE_DEPTH 32
#define MAX_SHADER_PATHS 32

static char* find_file(const char** includePaths, size_t numIncludePaths, const char* filename) {
    unsigned int i;
    size_t filenameSize = strlen(filename);
    char* path;
    FILE* test;

    for (i = 0; i < numIncludePaths; i++) {
        if (!(path = malloc(strlen(includePaths[i]) + 1 + filenameSize + 1))) {
            return 0;
        }
        sprintf(path, "%s/%s", includePaths[i], filename);
        if ((test = fopen(path, "r"))) {
            fclose(test);
            return path;
        }
        free(path);
    }
    return NULL;
}

static int append_code(const char* string, char** code, GLint* codeSize, unsigned int* allocSize) {
    size_t size = strlen(string), newSize;
    char* tmp;

    if ((size_t)(INT_MAX - *codeSize) < size) {
        fprintf(stderr, "Error: shader size is too large\n");
        return 0;
    }
    if (((size_t)*codeSize) + size >= ((size_t)*allocSize)) {
        if ((size_t)(INT_MAX - *allocSize) < (size + 1024)) {
            fprintf(stderr, "Error: shader size is too large\n");
            return 0;
        }
        newSize = *allocSize + size + 1024;
        if (!(tmp = realloc(*code, newSize))) {
            fprintf(stderr, "Error: memory allocation failed\n");
            return 0;
        }
        *code = tmp;
        *allocSize = newSize;
    }
    memcpy(*code + *codeSize, string, size);
    *codeSize += size;
    return 1;
}

GLuint shader_find_compile(const char* filename, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines) {
    char* path;
    GLuint shader = 0;
    if ((path = find_file(includePaths, numIncludePaths, filename))) {
        shader = shader_compile(path, type, includePaths, numIncludePaths, defines, numDefines);
        free(path);
    }
    return shader;
}

GLuint shader_compile(const char* path, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines) {
    FILE* fd;
    GLuint shader = 0;

    if (!(fd = fopen(path, "r"))) {
        fprintf(stderr, "Error: shader file %s not found\n", path);
    } else {
        shader = shader_compile_fd(fd, path, type, includePaths, numIncludePaths, defines, numDefines);
        fclose(fd);
    }

    return shader;
}

GLuint shader_compile_fd(FILE* fd, const char* pathInfo, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines) {
    char buffer[2048], lbuffer[64];
    struct File {
        FILE* fd;
        unsigned long line;
        unsigned int pathNum;
    } files[MAX_INCLUDE_DEPTH];
    char* paths[MAX_SHADER_PATHS];
    char* code = NULL;
    char *ptr, *cur, *end, *dirEnd, *altPath;
    size_t n;
    unsigned long curFile = 0, curPath = 0, ifLevel = 0;
    unsigned int codeAllocSize = 0;
    int ok = 1, keepGoing, found, version, hasVersion = 0, pendingLineDirective = 0;
    GLuint shader = 0;
    GLint error, errorSize, codeSize = 0;
    char* errorString;

    paths[0] = (char*)pathInfo;
    files[0].line = 1;
    files[0].pathNum = 0;
    files[0].fd = fd;
    do {
        if (!files[curFile].fd) {
            files[curFile].line = 1;
            altPath = NULL;
            if (!(files[curFile].fd = fopen(paths[files[curFile].pathNum], "r"))) {
                if (curFile) {
                    fprintf(stderr, "Error: %s:%lu #include file not found %s\n", paths[files[curFile - 1].pathNum], files[curFile - 1].line, paths[files[curFile].pathNum]);
                } else {
                    fprintf(stderr, "Error: shader file %s not found\n", paths[files[curFile].pathNum]);
                }
                ok = 0; break;
            }
        }
        if (curFile || files[curFile].line > 1) {
            if (hasVersion) {
                sprintf(lbuffer, "#line %lu %u\n", files[curFile].line - 1, files[curFile].pathNum);
                if (!append_code(lbuffer, &code, &codeSize, &codeAllocSize)) {
                    ok = 0; break;
                }
            } else {
                pendingLineDirective = 1;
            }
        }
        keepGoing = 1;
        while (ok && keepGoing && fgets(buffer, sizeof(buffer), files[curFile].fd)) {
            for (ptr = buffer; *ptr == ' ' || *ptr == '\t'; ptr++);
            if (!ifLevel && !strncmp(ptr, "#include", 8) && (ptr[8] == ' ' || ptr[8] == '\t')) {
                for (dirEnd = end = paths[files[curFile].pathNum]; *end; end++) {
                    if (*end == '/') dirEnd = end + 1;
                }
                for (ptr += 9; *ptr == ' ' || *ptr == '\t'; ptr++);
                for (end = ptr; *end; end++);
                for (--end; end > ptr && (*end == ' ' || *end == '\t' || *end == '\n'); --end);
                for (cur = ptr + 1; cur < end - 1 && *cur != '"'; cur++);
                ok = 0;
                if (((*ptr != '"' || *end != '"') && (*ptr != '<' || *end != '>')) || end <= ptr || *cur == '"') {
                    fprintf(stderr, "Error: %s:%lu invalid #include directive\n", paths[files[curFile].pathNum], files[curFile].line);
                } else if (curFile + 1 >= MAX_INCLUDE_DEPTH) {
                    fprintf(stderr, "Error: %s:%lu #include depth limit reached\n", paths[files[curFile].pathNum], files[curFile].line);
                } else if (curPath + 1 >= MAX_SHADER_PATHS) {
                    fprintf(stderr, "Error: %s:%lu shader path number limit reached\n", paths[files[curFile].pathNum], files[curFile].line);
                } else if (!(cur = malloc((dirEnd - paths[files[curFile].pathNum]) + (end - ptr)))) {
                    fprintf(stderr, "Error: memory allocation failed\n");
                } else {
                    memcpy(cur, paths[files[curFile].pathNum], n = (dirEnd - paths[files[curFile].pathNum]));
                    memcpy(cur + n, ptr + 1, (end - ptr) - 1);
                    cur[n + (end - ptr) - 1] = 0;
                    files[++curFile].pathNum = ++curPath;
                    files[curFile].fd = NULL;
                    if (*ptr == '<' && (altPath = find_file(includePaths, numIncludePaths, cur + n))) {
                        paths[curPath] = altPath;
                        free(cur);
                    } else {
                        paths[curPath] = cur;
                    }
                    ok = 1;
                    keepGoing = 0;
                }
            } else {
                if ((!strncmp(ptr, "#if", 3) && (ptr[3] == ' ' || ptr[3] == '\t'))
                 || (!strncmp(ptr, "#ifdef", 6) && (ptr[6] == ' ' || ptr[6] == '\t'))
                 || (!strncmp(ptr, "#ifndef", 7) && (ptr[7] == ' ' || ptr[7] == '\t'))) {
                    ifLevel++;
                } else if (ifLevel && !strncmp(ptr, "#endif", 6) && (ptr[6] == ' ' || ptr[6] == '\t' || ptr[6] == '\n' || !ptr[6])) {
                    ifLevel--;
                }

                if ((version = !strncmp(ptr, "#version", 8) && (ptr[8] == ' ' || ptr[8] == '\t'))) {
                    if (!append_code(buffer, &code, &codeSize, &codeAllocSize)) {
                        ok = 0;
                    }
                    files[curFile].line++;
                    hasVersion = 1;
                    if (pendingLineDirective) {
                        sprintf(lbuffer, "#line %lu %u\n", files[curFile].line - 1, files[curFile].pathNum);
                        if (!append_code(lbuffer, &code, &codeSize, &codeAllocSize)) {
                            ok = 0; break;
                        }
                    }
                }

                if (ok && numDefines) {
                    while (numDefines) {
                        if (!append_code("#define ", &code, &codeSize, &codeAllocSize)
                         || !append_code(*defines++, &code, &codeSize, &codeAllocSize)
                         || (*defines && (!append_code(" ", &code, &codeSize, &codeAllocSize) || !append_code(*defines, &code, &codeSize, &codeAllocSize)))
                         || !append_code("\n", &code, &codeSize, &codeAllocSize)) {
                            ok = 0; break;
                        }
                        defines++;
                        numDefines--;
                    }
                    if (ok) {
                        if (hasVersion) {
                            sprintf(lbuffer, "#line %lu %u\n", files[curFile].line - 1, files[curFile].pathNum);
                            if (!append_code(lbuffer, &code, &codeSize, &codeAllocSize)) {
                                ok = 0; break;
                            }
                        } else {
                            pendingLineDirective = 1;
                        }
                    }
                }

                if (ok && !version) {
                    if (!append_code(buffer, &code, &codeSize, &codeAllocSize)) {
                        ok = 0;
                    }
                    files[curFile].line++;
                }
            }
        }
        if (!ok) break;
        if (!keepGoing) continue;
        if (!curFile) break;
        fclose(files[curFile].fd);
        curFile--;
        files[curFile].line++;
    } while (1);

#if 0
    code[codeSize] = 0;
    printf("\n===%s\n%s\n", pathInfo, code);
#endif

    while (curFile) {
        if (files[curFile].fd) {
            fclose(files[curFile].fd);
        }
        curFile--;
    }

    if (ok) {
        if (!(shader = glCreateShader(type))) {
            fprintf(stderr, "Error: failed to create shader\n");
        } else {
            glShaderSource(shader, 1, (const char**)&code, &codeSize);
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
            ok = (error == GL_TRUE);
            if (!ok) {
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorSize);
                if (!(errorString = malloc(errorSize))) {
                    fprintf(stderr, "Error: failed to compile '%s' and failed to retrieve the compilation error\n", pathInfo);
                } else {
                    glGetShaderInfoLog(shader, errorSize, &errorSize, errorString);
                    errorString[errorSize] = '\0';
                    fprintf(stderr, "Error: failed to compile '%s'\n", pathInfo);
                    found = 0;
                    for (cur = errorString; *cur; cur++) {
                        if (!found && *cur >= '0' && *cur <= '9') {
                            unsigned long pathNum = strtoul(cur, &ptr, 10);
                            if (pathNum <= curPath && (*ptr == ':' || *ptr == '(')) {
                                unsigned long line = strtoul(ptr + 1, &end, 10);
                                if ((*ptr == ':' && *end == ':') || (*ptr == ':' && *end == '(') || (*ptr == '(' && *end == ')')) {
                                    fprintf(stderr, "%s: %lu", paths[pathNum], line);
                                    cur = end - (*ptr == ':');
                                    found = 1;
                                    continue;
                                }
                            }
                        }
                        if (*cur == '\n') found = 0;
                        fputc(*cur, stderr);
                    }
                    free(errorString);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    while (curPath) {
        free(paths[curPath--]);
    }
    free(code);

    return shader;
}

GLuint shader_link(const GLuint* shaders, size_t numShaders) {
    char* errorString;
    size_t i;
    GLint error, size;
    GLuint prog = 0;

    if (!(prog = glCreateProgram())) {
        fprintf(stderr, "Error: failed to create program\n");
        return 0;
    }
    for (i = 0; i < numShaders; i++) {
        glAttachShader(prog, shaders[i]);
    }
    glBindAttribLocation(prog, LOCATION_VERTEX, "in_Vertex");
    glBindAttribLocation(prog, LOCATION_NORMAL, "in_Normal");
    glBindAttribLocation(prog, LOCATION_TEXCOORD, "in_TexCoord");
    glBindAttribLocation(prog, LOCATION_TANGENT, "in_Tangent");
    glBindAttribLocation(prog, LOCATION_BITANGENT, "in_Bitangent");
    glLinkProgram(prog);

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
    }
    for (i = 0; i < numShaders; i++) {
        glDetachShader(prog, shaders[i]);
    }
    if (error != GL_TRUE) {
        glDeleteProgram(prog);
        prog = 0;
    } else {
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Lights"), LIGHTS_UBO_BINDING);
    }

    return prog;
}

GLuint shader_find_compile_link_vertfrag(const char* vpath, const char* fpath, const char** includePaths, size_t numIncludePaths, const char** vdefines, size_t vNumDefines, const char** fdefines, size_t fNumDefines) {
    GLuint shaders[2] = {0, 0}, program = 0;

    shaders[0] = shader_find_compile(vpath, GL_VERTEX_SHADER, includePaths, numIncludePaths, vdefines, vNumDefines);
    shaders[1] = shader_find_compile(fpath, GL_FRAGMENT_SHADER, includePaths, numIncludePaths, fdefines, fNumDefines);
    if (shaders[0] && shaders[1]) program = shader_link(shaders, 2);
    if (shaders[0]) glDeleteShader(shaders[0]);
    if (shaders[1]) glDeleteShader(shaders[1]);
    return program;
}
