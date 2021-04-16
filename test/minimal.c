#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/render/vertex_array.h>
#include <3dmr/render/viewer.h>
#include <3dmr/material/solid.h>
#include <3dmr/mesh/box.h>
#include <3dmr/shaders.h>

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

int main(int argc, char** argv) {
    Mat4 model;
    Mat3 inv;
    struct Viewer* viewer = NULL;
    struct VertexArray* va = NULL;
    struct Material* mat = NULL;
    struct UniformBuffer camera, lights;
    struct SolidMaterialParams matParams;
    int hascam = 0, haslights = 0;

    tdmrShaderRootPath = TDMR_SHADERS_PATH_SRC;
    load_id4(model);
    load_id3(inv);
    solid_material_params_init(&matParams);
    material_param_set_vec3_elems(&matParams.color, 0, 0, 1);
    if (!(viewer = viewer_new(640, 480, "test"))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(va = mkcube()) || !(mat = solid_material_new(0, &matParams))) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else if (!(hascam = camera_buffer_object_gen(&camera)) || !(haslights = lights_buffer_object_gen(&lights))) {
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
        material_use(mat);
        material_set_matrices(mat, model, inv);
        while (p.running) {
            viewer_next_frame(viewer);
            vertex_array_render(va);
            viewer_process_events(viewer);
        }
    }
    if (hascam) uniform_buffer_del(&camera);
    if (haslights) uniform_buffer_del(&lights);
    free(mat);
    vertex_array_free(va);
    viewer_free(viewer);
    return 0;
}
