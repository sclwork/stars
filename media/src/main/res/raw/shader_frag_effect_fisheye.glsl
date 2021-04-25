#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

void main() {
    vec2 uv;
    float bind;
    vec2 p = gl_FragCoord.xy / u_TexSize.x;
    float prop = u_TexSize.x / u_TexSize.y;
    vec2 m = vec2(0.5, 0.5 / prop);
    vec2 d = p - m;
    float r = sqrt(dot(d, d));
    float power = (2.0 * 3.141592 / (2.0 * sqrt(dot(m, m)))) * 0.3;
    if (power > 0.0) bind = sqrt(dot(m, m));
    else {if (prop < 1.0) bind = m.x; else bind = m.y;}
    if (power > 0.0)
        uv = m + normalize(d) * tan(r * power) * bind / tan(bind * power);
    else if (power < 0.0)
        uv = m + normalize(d) * atan(r * -power * 10.0) * bind / atan(-power * bind * 10.0);
    else uv = p;

    uv = vec2(uv.x, uv.y * prop);
    if (u_Mirror) uv.x = 1.0-uv.x;
    uv.y = 1.0-uv.y;
    outColor = vec4(texture(s_Texture, uv).rgb,1.0);
}