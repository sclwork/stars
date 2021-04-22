#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// The texture() call accepts a mip level offset as an optional parameter, which
// allows one to sample from different LODs of the texture. Besides being handy in
// some special situations, it also allows you to fake (box) blur of textures without
// having to perform a blur youtself. This has been traditionally used in demos and
// games to fake deph ot field and other similar effects in a very cheap way.

void main() {
    float lod = (5.0 + 5.0*sin(u_Time))*step(v_texCoord.x, 1.0);
    outColor = texture(s_Texture, v_texCoord, lod);
}