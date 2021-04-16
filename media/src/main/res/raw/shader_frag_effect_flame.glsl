#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform float u_Time;

float noise(vec3 p) {
    vec3 i = floor(p);
    vec4 a = dot(i, vec3(1.0, 57.0, 21.0)) + vec4(0.0, 57.0, 21.0, 78.0);
    vec3 f = cos((p-i)*acos(-1.0))*(-0.5)+0.5;
    a = mix(sin(cos(a)*a),sin(cos(1.0+a)*(1.0+a)), f.x);
    a.xy = mix(a.xz, a.yw, f.y);
    return mix(a.x, a.y, f.z);
}

float sphere(vec3 p, vec4 spr) {
    return length(spr.xyz-p) - spr.w;
}

float flame(vec3 p) {
    float d = sphere(p*vec3(1.0,0.5,1.0), vec4(0.0,-1.0,0.0,1.0));
    return d + (noise(p+vec3(0.0,u_Time*2.0,0.0)) + noise(p*3.)*0.5)*0.25*(p.y);
}

float scene(vec3 p) {
    return min(100.0-length(p), abs(flame(p)));
}

vec4 raymarch(vec3 org, vec3 dir) {
    float d = 0.0, glow = 0.0, eps = 0.02;
    vec3  p = org;
    bool glowed = false;

    for(int i=0; i<64; i++) {
        d = scene(p) + eps;
        p += d * dir;
        if(d>eps) {
            if(flame(p) < 0.0) {
                glowed=true;
            }
            if(glowed) {
                glow = float(i)/64.0;
            }
        }
    }
    return vec4(p,glow);
}

void main() {
    vec4 cc = texture(s_Texture, v_texCoord);

    vec2 fragCoord = gl_FragCoord.xy;
    vec2 v = -3.0 + 6.0 * fragCoord.xy / u_TexSize.xy;
    v.x *= u_TexSize.x / u_TexSize.y;
    v.y += 1.5;
    vec3 org = vec3(0.0, -2.0, 4.0);
    vec3 dir = normalize(vec3(v.x*1.6, -v.y, -1.5));
    vec4 p = raymarch(org, dir);
    float glow = p.w;
    vec4 col = mix(vec4(1.0,0.5,0.1,1.0), vec4(0.1,0.5,1.0,1.0), p.y*0.02+0.4);
//    outColor = mix(vec4(0.0), col, pow(glow*2.0,4.0));
    outColor = mix(cc, mix(vec4(1.0,0.5,0.1,1.0),vec4(0.1,0.5,1.0,1.0),p.y*0.02+0.4), pow(glow*2.0,4.0));
}