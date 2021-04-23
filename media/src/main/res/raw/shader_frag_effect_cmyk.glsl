#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

#define DOTSIZE 1.48
#define MIN_S 2.5
#define MAX_S 19.0
#define SPEED 0.57

#define SST 0.888
#define SSQ 0.288

float R;
float S;

vec4 rgb2cmyki(in vec3 c) {
    float k = max(max(c.r, c.g), c.b);
    return min(vec4(c.rgb / k, k), 1.0);
}

vec3 cmyki2rgb(in vec4 c) {
    return c.rgb * c.a;
}

vec2 px2uv(in vec2 px) {
    return vec2(px / u_TexSize.xy);
}

vec2 grid(in vec2 px) {
    return px - mod(px,S);
}

vec4 ss(in vec4 v) {
    return smoothstep(SST-SSQ, SST+SSQ, v);
}

vec4 halftone(in vec2 fc,in mat2 m) {
    vec2 smp = (grid(m*fc) + 0.5*S) * m;
    float s = min(length(fc-smp) / (DOTSIZE*0.5*S), 1.0);
    vec2 uv = px2uv(smp+(0.5*u_TexSize.xy));
    if (u_Mirror) uv.x = 1.0-uv.x;
    uv.y = 1.0-uv.y;
    vec3 texc = texture(s_Texture, uv).rgb;
    texc = pow(texc, vec3(2.2)); // Gamma decode.
    vec4 c = rgb2cmyki(texc);
    return c+s;
}

mat2 rotm(in float r) {
    float cr = cos(r);
    float sr = sin(r);
    return mat2(cr,-sr,sr,cr);
}

void main() {
    vec2 fragCoord = gl_FragCoord.xy;
    float iTime = u_Time / 64.0;

    R = SPEED*0.333*iTime;
    S = MIN_S + (MAX_S-MIN_S) * (0.5 - 0.5*cos(SPEED*iTime));

    vec2 fc = fragCoord - (0.5*u_TexSize.xy);

    mat2 mc = rotm(R + radians(15.0));
    mat2 mm = rotm(R + radians(75.0));
    mat2 my = rotm(R);
    mat2 mk = rotm(R + radians(45.0));

    float k = halftone(fc, mk).a;
    vec3 c = cmyki2rgb(ss(vec4(
        halftone(fc, mc).r,
        halftone(fc, mm).g,
        halftone(fc, my).b,
        halftone(fc, mk).a
    )));

    c = pow(c, vec3(1.0/2.2)); // Gamma encode.
    outColor = vec4(c, 1.0);
}