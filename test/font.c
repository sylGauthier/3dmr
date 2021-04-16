#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <3dmr/render/vertex_array.h>
#include <3dmr/render/viewer.h>
#include <3dmr/material/solid.h>
#include <3dmr/mesh/quad.h>
#include <3dmr/shaders.h>
#include <3dmr/font/ttf.h>
#include <3dmr/font/text.h>

struct App {
    int running;
    Mat4 model;
    Mat3 inv;
    struct VertexArray* va;
    struct Material* mat;
    size_t tw, th;
};

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* d) {
    if (key == GLFW_KEY_ESCAPE) {
        ((struct App*)d)->running = 0;
    }
}

static void close_callback(struct Viewer* viewer, void* d) {
    ((struct App*)d)->running = 0;
}

static void resize_callback(struct Viewer* viewer, void* d) {
    struct App* app = d;
    glViewport(0, 0, viewer->width, viewer->height);
    load_id4(app->model);
    if (app->tw > app->th) {
        app->model[1][1] = ((double)app->th) / ((double)app->tw);
    } else {
        app->model[0][0] = ((double)app->tw) / ((double)app->th);
    }
    if (viewer->width < viewer->height) {
        app->model[1][1] *= ((double)viewer->width) / ((double)viewer->height);
    } else {
        app->model[0][0] *= ((double)viewer->height) / ((double)viewer->width);
    }
    material_set_matrices(app->mat, app->model, app->inv);
}

struct VertexArray* mkquad(void) {
    struct Mesh mesh;
    struct VertexArray* va = NULL;
    if (make_quad(&mesh, 2, 2)) {
        va = vertex_array_new(&mesh);
        mesh_free(&mesh);
    }
    return va;
}

int main(int argc, char** argv) {
    struct App app;
    struct Viewer* viewer = NULL;
    struct SolidMaterialParams matParams;
    struct TTF ttf;
    struct Character* text = NULL;
    size_t nchars = 0, mapWidth, mapHeight = 32;
    GLuint tex = 0;
    int err = 1, hasttf = 0, isTTC = (argc > 3);

    if (argc != 3 && argc != 4) {
        fputs("Usage: test/font font.ttf text\n", stderr);
        fputs("       test/font font.ttc fnum text\n", stderr);
        return 1;
    }
    tdmrShaderRootPath = TDMR_SHADERS_PATH_SRC;
    load_id4(app.model);
    load_id3(app.inv);
    app.va = NULL;
    app.mat = NULL;
    solid_material_params_init(&matParams);
    if (!(hasttf = (isTTC ? ttc_load(argv[1], strtoul(argv[2], NULL, 10), &ttf) : ttf_load(argv[1], &ttf)))) {
        fputs("Error: failed to read font file\n", stderr);
    } else if (!ttf_load_chars(&ttf, argv[2 + isTTC], &text, &nchars, NULL)) {
        fputs("Error: failed to load characters\n", stderr);
    } else if (!(viewer = viewer_new(640, 480, "test"))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(tex = text_to_sdm_texture(text, nchars, mapHeight, &mapWidth))) {
        fprintf(stderr, "Error: failed to create texture\n");
    } else if (!(app.va = mkquad())
#if 0
            /* this shows the distance map */
            || (material_param_set_vec3_texture(&matParams.color, tex), !(app.mat = solid_overlay_material_new(app.va->flags, &matParams)))) {
#else
            /* while this shows the alpha-tested text */
            || (matParams.alpha.mode = ALPHA_TEST, material_param_set_float_texture(&matParams.alpha.alpha, tex), !(app.mat = solid_overlay_material_new(app.va->flags, &matParams)))) {
#endif
        fprintf(stderr, "Error: failed to create quad\n");
    } else {
        printf("Texture is %lu by %lu\n", mapWidth, mapHeight);
        err = 0;
        app.running = 1;
        app.tw = mapWidth;
        app.th = mapHeight;
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->resize_callback = resize_callback;
        viewer->callbackData = &app;
        material_use(app.mat);
        resize_callback(viewer, &app);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        while (app.running) {
            viewer_next_frame(viewer);
            vertex_array_render(app.va);
            viewer_process_events(viewer);
        }
    }
    if (hasttf) ttf_free(&ttf);
    while (nchars > 0) character_free(text + --nchars);
    free(text);
    if (tex) glDeleteTextures(1, &tex);
    free(app.mat);
    vertex_array_free(app.va);
    viewer_free(viewer);
    return err;
}
