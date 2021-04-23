#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

// https://www.shadertoy.com/view/MsS3Wc
// HSV to RGB conversion
vec3 hsv2rgb_smooth(in vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

vec2 viewport(vec2 p) {
    return p/(u_TexSize.xy);
}

vec3 sampleImage(vec2 coord){
    vec2 uv = viewport(coord);
    if (u_Mirror) uv.x = 1.0-uv.x;
    uv.y = 1.0-uv.y;
    return pow(max(texture(s_Texture,uv).rgb,0.0), vec3(2.2));;
}

float kernel(int a,int b){
    return float(a)*exp(-float(a*a + b*b)/36.0)/6.0;
}

void main() {
    vec3 rgb = sampleImage(gl_FragCoord.xy);

    vec3 col = vec3(0.0);
    vec3 colX = vec3(0.0);
    vec3 colY = vec3(0.0);
    float coeffX = 0.0;
    float coeffY = 0.0;

    for(int i = -6; i <= 6; i++) {
        for(int j = -6; j <= 6; j++) {
            coeffX = kernel(i,j);
            coeffY = kernel(j,i);
            col = sampleImage(gl_FragCoord.xy+vec2(i,j));
            colX += coeffX*col;
            colY += coeffY*col;
        }

    }

    vec3 derivative = sqrt((colX*colX + colY*colY))/(6.0*6.0);
    float angle = atan(dot(colY,vec3(0.2126,0.7152,0.0722)),dot(colX,vec3(0.2126,0.7152,0.0722)))/(2.0*3.14159265359) + u_Time/64.0*(1.0-0.5)/2.0;
    vec3 derivativeWithAngle = hsv2rgb_smooth(vec3(angle,1.0,pow(dot(derivative,vec3(0.2126,0.7152,0.0722))*3.0,3.0)*5.0));

    rgb = mix(derivative,rgb,0.5);
    rgb = mix(derivativeWithAngle,rgb,0.5);
    rgb = pow(max(rgb,0.0) , vec3(1.0/2.2) );

    outColor = vec4(rgb, 1.0);
}