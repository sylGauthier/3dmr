#version 130

#include "phong.glsl"

in vec3 surfelPosition;
in vec3 surfelNormal;
in vec2 coordTexture;
out vec4 out_Color;

uniform sampler2D tex;
uniform vec3 cameraPosition;

void main()
{
    out_Color = texture(tex, coordTexture) * vec4(phong_light_factor(cameraPosition, surfelPosition, surfelNormal), 1.0);
}
