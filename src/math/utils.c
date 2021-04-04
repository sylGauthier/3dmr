#include <string.h>

#include <3dmr/math/utils.h>
#include <3dmr/math/quaternion.h>

static int extract_scale(Vec3 scale, Mat4 t) {
    scale[0] = norm3(t[0]);
    scale[1] = norm3(t[1]);
    scale[2] = norm3(t[2]);
    if (!(scale[0] && scale[1] && scale[2])) return 0;
    scale3v(t[0], 1 / scale[0]);
    scale3v(t[1], 1 / scale[1]);
    scale3v(t[2], 1 / scale[2]);
    return 1;
}

int mat4toposrotscale(RESTRICT_MAT4(src), RESTRICT_VEC3(pos), RESTRICT_VEC4(rot), RESTRICT_VEC3(scale)) {
    if (!extract_scale(scale, src)) {
        return 0;
    }
    quaternion_from_mat4(rot, MAT_CONST_CAST(src));
    memcpy(pos, src[3], sizeof(Vec3));
    return 1;
}

