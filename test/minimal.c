#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/init.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/vertex_array.h>
#include <game/render/viewer.h>
#include <game/material/solid.h>
#include <game/mesh/box.h>

struct CallbackParam {
    int running;
    struct Camera* camera;
    GLuint uboCamera;
};

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* d) {
    if (key == GLFW_KEY_ESCAPE) {
        ((struct CallbackParam*)d)->running = 0;
    }
}

static void close_callback(struct Viewer* viewer, void* d) {
    ((struct CallbackParam*)d)->running = 0;
}

static void resize_callback(struct Viewer* viewer, void* d) {
    ((struct CallbackParam*)d)->camera->ratio = ((float)viewer->width) / ((float)viewer->height);
    camera_update_projection(((struct CallbackParam*)d)->camera);
    camera_buffer_object_update_projection(MAT_CONST_CAST(((struct CallbackParam*)d)->camera->projection), ((struct CallbackParam*)d)->uboCamera);
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
    struct Camera camera;
    Mat4 model;
    Mat3 inv;
    struct Viewer* viewer = NULL;
    struct VertexArray* va = NULL;
    struct Material* mat = NULL;
    GLuint ubos[2] = {0, 0};

    load_id4(model);
    load_id3(inv);
    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init library\n");
    } else if (!(viewer = viewer_new(640, 480, "test"))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(va = mkcube()) || !(mat = mkmat())) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else if (!(ubos[0] = camera_buffer_object()) || !(ubos[1] = lights_buffer_object())) {
        fprintf(stderr, "Error: failed to create UBOs\n");
    } else {
        struct CallbackParam p;
        p.running = 1;
        p.camera = &camera;
        p.uboCamera = ubos[0];
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->resize_callback = resize_callback;
        viewer->callbackData = &p;
        camera_load_default(&camera, 640.0 / 480.0);
        camera_buffer_object_update(&camera, ubos[0]);
        lights_buffer_object_zero_init(ubos[1]);
        while (p.running) {
            viewer_next_frame(viewer);
            vertex_array_render(va, mat, model, inv);
            viewer_process_events(viewer);
        }
    }
    glDeleteBuffers(2, ubos);
    free(mat);
    vertex_array_free(va);
    viewer_free(viewer);
    game_free();
    return 0;
}
