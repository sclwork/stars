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
    vec4 cc = texture(s_Texture, tc);
    // move to center
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 p = (2.0*fragCoord-u_TexSize.xy)/min(u_TexSize.y,u_TexSize.x);
    // background color
    // vec3 bcol = vec3(1.0,0.8,0.8)*(1.0-0.38*length(p));
    vec3 bcol = vec3(cc.r, cc.g, cc.b);
    // animate
    float tt = u_Time;
    float ss = pow(tt,.2)*0.5 + 0.5;
    ss = 1.0 + ss*0.5*sin(tt*6.2831*3.0 + p.y*0.5)*exp(-tt*4.0);
    p *= vec2(0.5,1.5) + ss*vec2(0.5,-0.5);
    // shape
    p.y -= 0.25;
    float a = atan(p.x,p.y) / 3.141592653;
    float r = length(p);
    float h = abs(a);
    float d = (13.0*h - 22.0*h*h + 10.0*h*h*h)/(6.0-5.0*h);
    // color
    float s = 0.75 + 0.75*p.x;
    s *= 1.0-0.4*r;
    s = 0.3 + 0.7*s;
    s *= 0.5+0.5*pow(1.0-clamp(r/d, 0.0, 1.0), 0.1);
    vec3 hcol = vec3(1.0,0.5*r,0.3)*s;
    vec3 col = mix(bcol, hcol, smoothstep(-0.06, 0.06, d-r));
    outColor = cc * 0.5 + vec4(col,1.0) * 0.5;
}