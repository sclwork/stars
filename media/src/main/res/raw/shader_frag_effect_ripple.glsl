#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform int u_FaceCount;
uniform vec4 u_FaceRect;
uniform float u_Time;
uniform float u_Boundary;
uniform vec4 u_RPoint;
uniform vec2 u_ROffset;

vec2 ripple(vec2 tc, float of, float cx, float cy) {
    float ratio = u_TexSize.y / u_TexSize.x;
    vec2 texCoord = tc * vec2(1.0, ratio);
    vec2 touchXY = vec2(cx, cy) * vec2(1.0, ratio);
    float distance = distance(texCoord, touchXY);
    if ((u_Time - u_Boundary) > 0.0
    && (distance <= (u_Time + u_Boundary))
    && (distance >= (u_Time - u_Boundary))) {
        float x = (distance - u_Time);
        float moveDis=of*x*(x-u_Boundary)*(x+u_Boundary);
        vec2 unitDirectionVec = normalize(texCoord - touchXY);
        texCoord = texCoord + (unitDirectionVec * moveDis);
    }
    texCoord = texCoord / vec2(1.0, ratio);
    return texCoord;
}

void main() {
    float fx = u_FaceRect.x / u_TexSize.x;
    float fy = u_FaceRect.y / u_TexSize.y;
    float fz = u_FaceRect.z / u_TexSize.x;
    float fw = u_FaceRect.w / u_TexSize.y;
    float cx = (fz + fx) / 2.0;
    float cy = (fw + fy) / 2.0;
    vec2 tc = ripple(v_texCoord, 20.0, cx, cy);
    tc=ripple(tc,u_ROffset.x,u_RPoint.x/u_TexSize.x,u_RPoint.y/u_TexSize.y);
    tc=ripple(tc,u_ROffset.y,u_RPoint.z/u_TexSize.x,u_RPoint.w/u_TexSize.y);
    outColor = texture(s_Texture, tc);
}