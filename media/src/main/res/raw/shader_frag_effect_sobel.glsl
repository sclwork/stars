#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

// Basic sobel filter implementation
// Jeroen Baert - jeroen.baert@cs.kuleuven.be
//
// www.forceflow.be


// Use these parameters to fiddle with settings
//const float step = 1.0;

float intensity(vec4 color) {
    return sqrt((color.x*color.x)+(color.y*color.y)+(color.z*color.z));
}

vec4 sobel(float stepx, float stepy, vec2 center) {
    // get samples around pixel
    float tleft  = intensity(texture(s_Texture,center + vec2(-stepx,stepy)));
    float left   = intensity(texture(s_Texture,center + vec2(-stepx,0.0)));
    float bleft  = intensity(texture(s_Texture,center + vec2(-stepx,-stepy)));
    float top    = intensity(texture(s_Texture,center + vec2(0.0,stepy)));
    float bottom = intensity(texture(s_Texture,center + vec2(0.0,-stepy)));
    float tright = intensity(texture(s_Texture,center + vec2(stepx,stepy)));
    float right  = intensity(texture(s_Texture,center + vec2(stepx,0.0)));
    float bright = intensity(texture(s_Texture,center + vec2(stepx,-stepy)));

    // Sobel masks (see http://en.wikipedia.org/wiki/Sobel_operator)
    //        1 0 -1     -1 -2 -1
    //    X = 2 0 -2  Y = 0  0  0
    //        1 0 -1      1  2  1

    // You could also use Scharr operator:
    //        3 0 -3        3 10   3
    //    X = 10 0 -10  Y = 0  0   0
    //        3 0 -3        -3 -10 -3

    float x =  tleft + 2.0*left + bleft  - tright - 2.0*right  - bright;
    float y = -tleft - 2.0*top  - tright + bleft  + 2.0*bottom + bright;
    float color = sqrt((x*x) + (y*y));
    return vec4(color,color,color, 1.0);
}

void main() {
    float stepx = 1.0 / u_TexSize.x;
    float stepy = 1.0 / u_TexSize.y;
    outColor = sobel(stepx, stepy, v_texCoord);
}