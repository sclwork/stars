#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

void main() {
    vec2 uv = v_texCoord.xy;
    uv.x += sin(uv.y*10.0+u_Time/16.0)/10.0;
    outColor = texture(s_Texture, uv);
}