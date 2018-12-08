#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <GL/glew.h>
#include <game/asset_manager.h>

#define MAX_INCLUDE_DEPTH 32
#define MAX_SHADER_PATHS 32

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

static int append_code(const char* string, char** code, unsigned int* codeSize, unsigned int* allocSize) {
    size_t size = strlen(string), newSize;
    char* tmp;

    if (((size_t)*codeSize) + size >= ((size_t)*allocSize)) {
        newSize = *allocSize + size + 1024;
        if (newSize > (size_t)UINT_MAX) {
            fprintf(stderr, "Error: shader size is too large\n");
            return 0;
        }
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

static GLuint compile(const char* path, GLenum type) {
    char buffer[2048];
    struct File {
        FILE* fd;
        unsigned long line;
        unsigned int pathNum;
        char* altPath;
    } files[MAX_INCLUDE_DEPTH];
    char* paths[MAX_SHADER_PATHS];
    char* code = NULL;
    char *ptr, *cur, *end, *dirEnd, *altPath;
    size_t n;
    unsigned long curFile = 0, curPath = 0, ifLevel = 0;
    unsigned int codeSize = 0, codeAllocSize = 0;
    int ok = 1, keepGoing, found;
    GLuint shader = 0;
    GLint error, errorSize;
    char* errorString;

    if (!(paths[0] = malloc(strlen(path) + 1))) {
        fprintf(stderr, "Error: memory allocation error\n");
        return 0;
    }
    strcpy(paths[0], path);
    files[0].pathNum = 0;
    files[0].fd = NULL;
    files[0].altPath = NULL;
    do {
        if (!files[curFile].fd) {
            files[curFile].line = 1;
            altPath = NULL;
            if (!(files[curFile].fd = fopen(paths[files[curFile].pathNum], "r"))
             && (!(altPath = files[curFile].altPath) || !(files[curFile].fd = fopen(altPath, "r")))) {
                if (curFile) {
                    fprintf(stderr, "Error: %s:%lu #include file not found %s\n", paths[files[curFile - 1].pathNum], files[curFile - 1].line, paths[files[curFile].pathNum]);
                } else {
                    fprintf(stderr, "Error: shader file %s not found\n", path);
                }
                ok = 0; break;
            }
            if (altPath) {
                free(paths[files[curFile].pathNum]);
                paths[files[curFile].pathNum] = altPath;
            } else if (files[curFile].altPath) {
                free(files[curFile].altPath);
                files[curFile].altPath = NULL;
            }
        }
        if (curFile || files[curFile].line > 1) {
            sprintf(buffer, "#line %lu %u\n", files[curFile].line - 1, files[curFile].pathNum);
            if (!append_code(buffer, &code, &codeSize, &codeAllocSize)) {
                ok = 0; break;
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
                    paths[++curPath] = cur;
                    files[++curFile].pathNum = curPath;
                    files[curFile].fd = NULL;
                    if (*ptr == '<' && (altPath = asset_manager_find_file(cur + n))) {
                        files[curFile].altPath = cur;
                        paths[curPath] = altPath;
                    } else {
                        files[curFile].altPath = NULL;
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
                if (!append_code(buffer, &code, &codeSize, &codeAllocSize)) {
                    ok = 0;
                }
                files[curFile].line++;
            }
        }
        if (!ok) break;
        if (!keepGoing) continue;
        if (!curFile) break;
        curFile--;
        files[curFile].line++;
    } while (1);

#if 0
    code[codeSize] = 0;
    printf("\n===%s\n%s\n", path, code);
#endif

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
                    fprintf(stderr, "Error: failed to compile '%s' and failed to retrieve the compilation error\n", path);
                } else {
                    glGetShaderInfoLog(shader, errorSize, &errorSize, errorString);
                    fprintf(stderr, "Error: failed to compile '%s'\n", path);
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
                    errorString[errorSize] = '\0';
                    free(errorString);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    do {
        free(paths[curPath]);
    } while (curPath--);

    return shader;
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
