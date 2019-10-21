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
    struct UniformBuffer* camera;
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
    Mat4 projection;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_projection(((float)viewer->width) / ((float)viewer->height), 1.04, 0.1, 2000, projection);
    camera_buffer_object_update_projection(((struct CallbackParam*)d)->camera, MAT_CONST_CAST(projection));
    uniform_buffer_send(((struct CallbackParam*)d)->camera);
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
    if ((mat = solid_material_new(0))) {
        material_param_set_vec3_elems(&mat->color, 0, 0, 1);
    }
    return (struct Material*)mat;
}

int main(int argc, char** argv) {
    Mat4 model;
    Mat3 inv;
    struct Viewer* viewer = NULL;
    struct VertexArray* va = NULL;
    struct Material* mat = NULL;
    struct UniformBuffer camera, lights;

    load_id4(model);
    load_id3(inv);
    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init library\n");
    } else if (!(viewer = viewer_new(640, 480, "test"))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(va = mkcube()) || !(mat = mkmat())) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else if (!camera_buffer_object_gen(&camera) || !lights_buffer_object_gen(&lights)) {
        fprintf(stderr, "Error: failed to create UBOs\n");
    } else {
        struct CallbackParam p;
        p.running = 1;
        p.camera = &camera;
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->resize_callback = resize_callback;
        viewer->callbackData = &p;
        camera_buffer_object_default_init(&camera, 640.0 / 480.0);
        lights_buffer_object_zero_init(&lights);
        uniform_buffer_send(&camera);
        uniform_buffer_send(&lights);
        while (p.running) {
            viewer_next_frame(viewer);
            vertex_array_render(va, mat, model, inv);
            viewer_process_events(viewer);
        }
    }
    uniform_buffer_del(&camera);
    uniform_buffer_del(&lights);
    free(mat);
    vertex_array_free(va);
    viewer_free(viewer);
    game_free();
    return 0;
}
