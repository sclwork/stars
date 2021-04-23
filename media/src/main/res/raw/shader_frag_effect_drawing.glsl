#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

/*
    Author: Daniel Taylor
	License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

	Tried my hand at a sketch-looking shader.

	I'm sure that someone has used this exact method before, but oh well. I like to
	think that this one is very readable (aka I'm not very clever with optimizations).
	There's little noise in the background, which is a good sign, however it's easy to
	create a scenerio that tricks it (the 1961 Commerical video is a good example).
	Also, text (or anything thin) looks really bad on it, don't really know how to fix
	that.

	Also, if the Shadertoy devs are reading this, the number one feature request that
	I have is a time slider. Instead of waiting for the entire video to loop back to
	the end, be able to fast forward to a specific part. It'd really help, I swear.

	Previous work:
	https://www.shadertoy.com/view/XtVGD1 - the grandaddy of all sketch shaders, by flockaroo
*/

#define PI2 6.28318530717959

#define RANGE 16.0
#define STEP 2.0
#define ANGLENUM 4.0

// Grayscale mode! This is for if you didn't like drawing with colored pencils as a kid
//#define GRAYSCALE

// Here's some magic numbers, and two groups of settings that I think looks really nice.
// Feel free to play around with them!

#define MAGIC_GRAD_THRESH 0.01

//// Setting group 1:
//#define MAGIC_SENSITIVITY     4.0
//#define MAGIC_COLOR           1.0

// Setting group 2:
#define MAGIC_SENSITIVITY     10.0
#define MAGIC_COLOR           0.5

//---------------------------------------------------------
// Your usual image functions and utility stuff
//---------------------------------------------------------
vec4 getCol(vec2 pos) {
    vec2 uv = pos / u_TexSize.xy;
    if (u_Mirror) uv.x = 1.0-uv.x;
    uv.y = 1.0-uv.y;
    return texture(s_Texture, uv);
}

float getVal(vec2 pos) {
    vec4 c=getCol(pos);
    return dot(c.xyz, vec3(0.2126, 0.7152, 0.0722));
}

vec2 getGrad(vec2 pos, float eps) {
    vec2 d=vec2(eps,0);
    return vec2(
    getVal(pos+d.xy)-getVal(pos-d.xy),
    getVal(pos+d.yx)-getVal(pos-d.yx)
    )/eps/2.;
}

void pR(inout vec2 p, float a) {
    p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

float absCircular(float t) {
    float a = floor(t + 0.5);
    return mod(abs(a - t), 1.0);
}

void main() {
    vec2 pos = gl_FragCoord.xy;
    float weight = 1.0;

    for (float j = 0.; j < ANGLENUM; j += 1.0) {
        vec2 dir = vec2(1.0, 0.0);
        pR(dir, j * PI2 / (2.0 * ANGLENUM));

        vec2 grad = vec2(-dir.y, dir.x);

        for (float i = -RANGE; i <= RANGE; i += STEP) {
            vec2 pos2 = pos + normalize(dir)*i;

            // video texture wrap can't be set to anything other than clamp  (-_-)
            if (pos2.y < 0.0 || pos2.x < 0.0 || pos2.x > u_TexSize.x || pos2.y > u_TexSize.y)
                continue;

            vec2 g = getGrad(pos2, 1.0);
            if (length(g) < MAGIC_GRAD_THRESH)
                continue;

            weight -= pow(abs(dot(normalize(grad), normalize(g))), MAGIC_SENSITIVITY) / floor((2.0 * RANGE + 1.0) / STEP) / ANGLENUM;
        }
    }

    #ifndef GRAYSCALE
        vec4 col = getCol(pos);
    #else
        vec4 col = vec4(getVal(pos));
    #endif

    vec4 background = mix(col, vec4(1.0), MAGIC_COLOR);

    // I couldn't get this to look good, but I guess it's almost obligatory at this point...
    /*float distToLine = absCircular(fragCoord.y / (u_TexSize.y/8.));
    background = mix(vec4(0.6,0.6,1,1), background, smoothstep(0., 0.03, distToLine));*/


    // because apparently all shaders need one of these. It's like a law or something.
    float r = length(pos - u_TexSize.xy*.5) / u_TexSize.x;
    float vign = 1.0 - r*r*r;

//    vec4 a = texture(s_Texture, pos/u_TexSize.xy);
//    outColor = vign * mix(vec4(0.0), background, weight) + a.xxxx/25.0;
    outColor = vign * mix(vec4(0.0), background, weight);
//    outColor = getCol(pos);
}