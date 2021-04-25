#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

vec2 barrelDistortion(vec2 coord, float amt) {
    vec2 cc = coord - 0.5;
    float dist = dot(cc, cc);
    return coord + cc * dist * amt;
}

void main() {
    vec2 uv=(gl_FragCoord.xy/u_TexSize.xy*0.5)+0.25;
    if (u_Mirror) uv.x = 1.0-uv.x;
    uv.y = 1.0-uv.y;

    vec4 a1 =texture(s_Texture, barrelDistortion(uv,0.0));
    vec4 a2 =texture(s_Texture, barrelDistortion(uv,0.2));
    vec4 a3 =texture(s_Texture, barrelDistortion(uv,0.4));
    vec4 a4 =texture(s_Texture, barrelDistortion(uv,0.6));

    vec4 a5 =texture(s_Texture, barrelDistortion(uv,0.8));
    vec4 a6 =texture(s_Texture, barrelDistortion(uv,1.0));
    vec4 a7 =texture(s_Texture, barrelDistortion(uv,1.2));
    vec4 a8 =texture(s_Texture, barrelDistortion(uv,1.4));

    vec4 a9 =texture(s_Texture, barrelDistortion(uv,1.6));
    vec4 a10=texture(s_Texture, barrelDistortion(uv,1.8));
    vec4 a11=texture(s_Texture, barrelDistortion(uv,2.0));
    vec4 a12=texture(s_Texture, barrelDistortion(uv,2.2));

    vec4 tx=(a1+a2+a3+a4+a5+a6+a7+a8+a9+a10+a11+a12)/12.0;
    outColor = vec4(tx.rgb,1.0);
}