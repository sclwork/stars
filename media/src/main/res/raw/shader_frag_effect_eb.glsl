#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

/**
 * Edge Detection: 834144373's https://www.shadertoy.com/view/MdGGRt
 * Bilateral Filter: https://www.shadertoy.com/view/4dfGDH
 */

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15

const mat4 kernel = mat4(
    0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565,
    0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044,
    0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216, 0.0);

float sigmoid(float a, float f) {
    return 1.0 / (1.0 + exp(-f * a));
}

void main() {
    float edgeStrength =  length(fwidth(texture(s_Texture, v_texCoord)));
    edgeStrength = sigmoid(edgeStrength - 0.2, 15.0);
    outColor = vec4(vec3(edgeStrength), 1.0);
}