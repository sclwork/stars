#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

struct Rectangle {
    vec3 v1;
    vec3 v2;
    vec3 v3;
};

vec3 CalcNormal(Rectangle A) {
    vec3 first = A.v1 - A.v2;
    vec3 second = A.v2 - A.v3;

    return cross(first,second);
}

vec3 Intersect(vec3 B, vec3 r, vec3 A, vec3 p) {
    float u=0.0;
    if (r.x*p.x+r.y*p.y+r.z*p.z!=0.0) {
        u =  (dot(A,p)-dot(B,p))/(dot(r,p));
    }

    return B+r*u;
}

vec2 Calc_ul(vec3 B, vec3 C, vec3 r, vec3 s) {
    float l = (C.y*r.x-B.y*r.x-C.x*r.y+B.x*r.y)/(s.y-s.x*r.y);
    float u = 0.0;
    if (r.x==0.0) {
        u=0.0;
    } else {
        u = (C.x-B.x-l*s.x)/r.x;
    }

    return vec2(u,l);
}

vec3 GetScreenPixelColor(vec2 ul) {
    float x = mod(ul.x * u_TexSize.x / (u_TexSize.y/u_TexSize.x) * 3.0,3.0);
    vec3 tex = texture(s_Texture, vec2(ul.x,ul.y) ).rgb;
    return mix(mix(vec3(tex.r,0.0,0.0),vec3(0.0,tex.g,0.0),step(1.0,x)), vec3(0.0,0.0,tex.b),step(2.0,x));
}

void main() {
    vec2 q = v_texCoord;
    float iTime = u_Time/64.0;

    vec2 rect_offset = vec2(-0.5, -0.5);
    float zoom = 0.9+sin(iTime*0.3)*0.55;

    Rectangle rect;
    rect.v1 = vec3(0.1+rect_offset.x, 0.1+rect_offset.y, 0.4+sin(iTime*1.5)*0.15);
    rect.v2 = vec3(0.1+rect_offset.x, 0.9+rect_offset.y, 0.5+sin(iTime*1.5)*0.15);
    rect.v3 = vec3(0.9+rect_offset.x, 0.9+rect_offset.y, 0.5+cos(iTime*1.0)*0.15);

    float bu = abs(rect.v2.x - rect.v3.x);
    float bl = abs(rect.v1.y - rect.v2.y);


    vec3 rect_normal_vector = CalcNormal(rect);
    rect_normal_vector = normalize(rect_normal_vector);

    vec3 plane_vector1 = normalize(rect.v2 - rect.v1);
    vec3 plane_vector2 = normalize(rect.v3 - rect.v2);


    vec3 screen_vector = vec3( q.x/10.0 , q.y/10.0 , -1);
    vec3 camera = vec3( q.x-0.5, q.y+0.5, zoom);
    vec3 ray = camera - screen_vector;
    vec3 intersection = Intersect(camera,ray,rect.v3,rect_normal_vector);

    vec2 ul = Calc_ul(rect.v2, intersection, plane_vector2, plane_vector1);
    vec3 new_col = vec3(0.0, 0.0, 0.0);
    vec3 old_col = vec3(0.0, 0.0, 0.0);

    if (ul.x>0.0 && ul.x<1.0 && ul.y>0.0 && ul.y<1.0) {
        float count=0.0;

        for (float i=-1.0;i<1.0;i+=0.33) {
            new_col += GetScreenPixelColor(vec2(ul.x+i/u_TexSize.x,ul.y));
            count = count + 1.0;
        }

        new_col = 3.0*new_col/count;
        old_col = texture(s_Texture, vec2(ul.x,ul.y)).rgb;
    }

    outColor = vec4(new_col,1.0);
}