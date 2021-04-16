#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform int u_FaceCount;
uniform vec4 u_FaceRect;

void main() {
    float fx = u_FaceRect.x / u_TexSize.x;
    float fy = u_FaceRect.y / u_TexSize.y;
    float fz = u_FaceRect.z / u_TexSize.x;
    float fw = u_FaceRect.w / u_TexSize.y;
    float cw = 0.5 / u_TexSize.x;
    float ch = 0.5 / u_TexSize.y;
    if (((v_texCoord.x > fx - cw && v_texCoord.x < fx + cw)
      || (v_texCoord.y > fy - ch && v_texCoord.y < fy + ch)
      || (v_texCoord.x > fz - cw && v_texCoord.x < fz + cw)
      || (v_texCoord.y > fw - ch && v_texCoord.y < fw + ch))
      && (v_texCoord.x > fx - cw && v_texCoord.x < fz + cw
       && v_texCoord.y > fy - ch && v_texCoord.y < fw + ch)
      && u_FaceCount > 0) {
        outColor = vec4(1.0, 1.0, 1.0, 1.0);
    } else {
        outColor = texture(s_Texture, v_texCoord);
    }
}