#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

void main() {
    vec3  col = texture(s_Texture, v_texCoord).rgb;
    float lum = dot(col,vec3(0.333));
    vec3 ocol = col;

    if(v_texCoord.x>0.5) {
        // right side: changes in luminance
        float f = fwidth(lum);
        col *= 1.5*vec3(clamp(1.0-8.0*f,0.0,1.0));
    } else {
        // bottom left: emboss
        vec3 nor = normalize(vec3(dFdx(lum), 64.0/u_TexSize.x, dFdy(lum)));
        if(v_texCoord.y<0.5) {
            float lig = 0.5 + dot(nor,vec3(0.7,0.2,-0.7));
            col = vec3(lig);
        }
        // top left: bump
        else {
            float lig = clamp(0.5 + 1.5*dot(nor,vec3(0.7,0.2,-0.7)), 0.0, 1.0);
            col *= vec3(lig);
        }
    }

    col *= smoothstep(0.003, 0.004, abs(v_texCoord.x-0.5));
    col *= 1.0 - (1.0-smoothstep(0.007, 0.008, abs(v_texCoord.y-0.5)))*(1.0-smoothstep(0.49,0.5,v_texCoord.x));
    col = mix(col, ocol, pow(0.5 + 0.5*sin(u_Time/64.0), 4.0));

    outColor = vec4(col, 1.0);
}