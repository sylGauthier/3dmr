#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/init.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/material/solid.h>
#include <game/mesh/box.h>

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* d) {
    if (key == GLFW_KEY_ESCAPE) {
        *(int*)d = 0;
    }
}

static void close_callback(struct Viewer* viewer, void* d) {
    *(int*)d = 0;
}

struct VertexArray* mkcube(void) {
    struct Mesh mesh;
    struct VertexArray* va = NULL;
    if (make_box(&mesh, 1, 1, 1)) {
        va = vertex_array_new(&mesh);
        mesh_free(&mesh);
    }
    return va;
}

struct Material* mkmat(void) {
    struct SolidMaterial* mat;
    if ((mat = solid_material_new(MAT_PARAM_CONSTANT))) {
        material_param_set_vec3_elems(&mat->color, 0, 0, 1);
    }
    return (struct Material*)mat;
}

int main(int argc, char** argv) {
    Mat4 model;
    Mat3 inv;
    struct GLObject glo;
    struct Viewer* viewer = NULL;
    struct VertexArray* va = NULL;
    struct Material* mat = NULL;
    GLuint camera = 0, lights = 0;
    int ret = 1;

    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init library\n");
        return 0;
    }
    load_id4(model);
    load_id3(inv);

    if (!(viewer = viewer_new(640, 480, "test"))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(va = mkcube())) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else if (!(mat = mkmat())) {
        fprintf(stderr, "Error: failed to create material\n");
    } else if (!(camera = camera_buffer_object()) || !(lights = lights_buffer_object())) {
        fprintf(stderr, "Error: failed to create UBOs\n");
    } else {
        int running = 1;
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->callbackData = &running;
        glo.vertexArray = va;
        glo.material = mat;
        camera_buffer_object_update(&viewer->camera, camera);
        {
            struct Lights l = {0};
            lights_buffer_object_update(&l, lights);
        }
        while (running) {
            viewer_next_frame(viewer);
            globject_render(&glo, model, inv);
            viewer_process_events(viewer);
        }
    }

    glDeleteBuffers(1, &camera);
    glDeleteBuffers(1, &lights);
    free(mat);
    vertex_array_free(va);
    viewer_free(viewer);
    game_free();
    return ret;
}
