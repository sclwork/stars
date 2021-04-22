#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform float u_Time;
uniform bool u_Mirror;

#define WIDTH 0.48
#define HEIGHT 0.36
#define CURVE 3.0
#define SMOOTH 0.004
#define SHINE 0.33

#define BEZEL_COL_A vec4(0.8, 0.8, 0.6, 0.0)
#define BEZEL_COL_B vec4(0.8, 0.8, 0.6, 0.0)

//#define REFLECTION_BLUR_ITERATIONS 5
//#define REFLECTION_BLUR_SIZE 0.04

// change these values to 0.0 to turn off individual effects
float vertJerkOpt = 0.0;
float vertMovementOpt = 0.0;
float bottomStaticOpt = 1.0;
float scalinesOpt = 1.0;
float rgbOffsetOpt = 1.0;
float horzFuzzOpt = 1.0;

// Noise generation functions borrowed from:
// https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                       -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step(x0.y, x0.x); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
    + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float staticV(vec2 uv) {
    float staticHeight = snoise(vec2(9.0,u_Time*1.2+3.0))*0.3+5.0;
    float staticAmount = snoise(vec2(1.0,u_Time*1.2-6.0))*0.1+0.3;
    float staticStrength = snoise(vec2(-9.75,u_Time*0.6-3.0))*2.0+2.0;
    return (1.0-step(snoise(vec2(5.0*pow(u_Time,2.0)+pow(uv.x*7.0,1.2),pow((mod(u_Time,100.0)+100.0)*uv.y*0.3+3.0,staticHeight))),staticAmount))*staticStrength;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 CurvedSurface(vec2 uv, float r) {
    return r * uv/sqrt(r * r - dot(uv, uv));
}

vec2 crtCurve(vec2 uv, float r, bool content, bool shine) {
    vec3 iResolution = vec3(u_TexSize.x*0.77, u_TexSize.x*0.77, 0.0);
    r = CURVE * r;
//    if (iMouse.z > 0.) r *= exp(0.5 - iMouse.y/iResolution.y);
    uv = (uv / iResolution.xy - 0.5);
    uv.x -= 0.15;
    uv.y -= 1.40;
    uv = uv / vec2(iResolution.y/iResolution.x, 1.0) * 2.0;
    uv = CurvedSurface(uv, r);
    if(content) uv *= 0.5 / vec2(WIDTH, HEIGHT);
    uv = (uv / 2.0) + 0.5;
//    if(!shine) if (iMouse.z > 0.) uv.x -= iMouse.x/iResolution.x - 0.5;
    return uv;
}

vec2 crtCurveB(vec2 uv, float r, bool content, bool shine) {
    vec3 iResolution = vec3(u_TexSize.x*0.77, u_TexSize.x*0.77, 0.0);
    r = CURVE * r;
    //    if (iMouse.z > 0.) r *= exp(0.5 - iMouse.y/iResolution.y);
    uv = (uv / iResolution.xy - 0.5);
    uv.x -= 0.15;
    uv.y -= 0.25;
    uv = uv / vec2(iResolution.y/iResolution.x, 1.0) * 2.0;
    uv = CurvedSurface(uv, r);
    if(content) uv *= 0.5 / vec2(WIDTH, HEIGHT);
    uv = (uv / 2.0) + 0.5;
    //    if(!shine) if (iMouse.z > 0.) uv.x -= iMouse.x/iResolution.x - 0.5;
    return uv;
}

float roundSquare(vec2 p, vec2 b, float r) {
    return length(max(abs(p)-b,0.0))-r;
}

// standard roundSquare
float stdRS(vec2 uv, float r) {
    return roundSquare(uv - 0.5, vec2(WIDTH, HEIGHT) + r, 0.05);
}

// Calculate normal to distance function and move along
// normal with distance to get point of reflection
vec2 borderReflect(vec2 p, float r) {
    float eps = 0.0001;
    vec2 epsx = vec2(eps,0.0);
    vec2 epsy = vec2(0.0,eps);
    vec2 b = (1.+vec2(r,r))* 0.5;
    r /= 3.0;

    p -= 0.5;
    vec2 normal = vec2(roundSquare(p-epsx,b,r)-roundSquare(p+epsx,b,r),
    roundSquare(p-epsy,b,r)-roundSquare(p+epsy,b,r))/eps;
    float d = roundSquare(p, b, r);
    p += 0.5;
    return p + d*normal;
}

vec4 boxA() {
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 c = vec4(0.0, 0.0, 0.0, 0.0);

    vec2 uvC = crtCurve(fragCoord, 1.0, true, false); 	// Content Layer
    vec2 uvS = crtCurve(fragCoord, 1.0, false, false);	// Screen Layer
    vec2 uvE = crtCurve(fragCoord, 1.25, false, false);	// Enclosure Layer

    // From my shader https://www.shadertoy.com/view/MtBXW3
//    const float ambient = 0.33;

    // Glass Shine
    vec2 uvSh = crtCurve(fragCoord, 1.0, false, true);
    c += max(0.0, SHINE - distance(uvSh, vec2(0.5, 1.0))) *
    smoothstep(SMOOTH/2.0, -SMOOTH/2.0, stdRS(uvS + vec2(0., 0.03), 0.0));

//    // Ambient
//    c += max(0.0, ambient - 0.5*distance(uvS, vec2(0.5,0.5))) *
//    smoothstep(SMOOTH, -SMOOTH, stdRS(uvS, 0.0));

    // Enclosure Layer
    uvSh = crtCurve(fragCoord, 1.25, false, true);
    vec4 b = vec4(0.0, 0.0, 0.0, 0.0);
    for(int i=0; i<12; i++)
    b += (clamp(BEZEL_COL_A + rand(uvSh+float(i))*0.05-0.025, 0.0, 1.0) +
    rand(uvE+1.0+float(i))*0.25 * cos((uvSh.x-0.5)*3.1415*1.5))/12.0;

    // Inner Border
    const float HHW = 0.5 * HEIGHT/WIDTH;

    c += b/3.0*(1.0 + smoothstep(HHW - 0.025, HHW + 0.025, abs(atan(uvS.x-0.5, uvS.y-0.5))/3.1415)
    + smoothstep(HHW + 0.025, HHW - 0.025, abs(atan(uvS.x-0.5, 0.5-uvS.y))/3.1415))*
    smoothstep(-SMOOTH, SMOOTH, stdRS(uvS, 0.0)) *
    smoothstep(SMOOTH, -SMOOTH, stdRS(uvE, 0.05));

    // Inner Border Shine
    c += (b - 0.4)*
    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.505), vec2(WIDTH, HEIGHT) + 0.05, 0.05)) *
    smoothstep(SMOOTH*2.0, -SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.495), vec2(WIDTH, HEIGHT) + 0.05, 0.05));

    // Outer Border
    c += b *
    smoothstep(-SMOOTH, SMOOTH, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.05, 0.05)) *
    smoothstep(SMOOTH, -SMOOTH, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

    // Outer Border Shine
    c += (b - 0.4)*
    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.495), vec2(WIDTH, HEIGHT) + 0.15, 0.05)) *
    smoothstep(SMOOTH*2.0, -SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.505), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

//    // Table and room
//    c += max(0.0, (1.0 - 2.0* fragCoord.y/iResolution.y)) * vec4(1.0, 1.0, 1.0, 0.0) *
//    smoothstep(-0.25, 0.25, roundSquare(uvC - vec2(0.5, -0.2), vec2(WIDTH+0.25, HEIGHT-0.15), 0.1)) *
//    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

    if (uvC.x > 0.0 && uvC.x < 1.0 && uvC.y > 0.0 && uvC.y < 1.0) {
        vec2 uv = v_texCoord; //fragCoord.xy/u_TexSize.xy;

        float jerkOffset = (1.0-step(snoise(vec2(u_Time*1.3,5.0)),0.8))*0.05;

        float fuzzOffset = snoise(vec2(u_Time*15.0,uv.y*80.0))*0.003;
        float largeFuzzOffset = snoise(vec2(u_Time*1.0,uv.y*25.0))*0.004;

        float vertMovementOn = (1.0-step(snoise(vec2(u_Time*0.2,8.0)),0.4))*vertMovementOpt;
        float vertJerk = (1.0-step(snoise(vec2(u_Time*1.5,5.0)),0.6))*vertJerkOpt;
        float vertJerk2 = (1.0-step(snoise(vec2(u_Time*5.5,5.0)),0.2))*vertJerkOpt;
        float yOffset = abs(sin(u_Time)*4.0)*vertMovementOn+vertJerk*vertJerk2*0.3;
        float y = mod(uv.y+yOffset,1.0);

        float staticVal = 0.0;
        float xOffset = (fuzzOffset + largeFuzzOffset) * horzFuzzOpt;

        for (float y = -1.0; y <= 1.0; y += 1.0) {
            float maxDist = 5.0/200.0;
            float dist = y/200.0;
            staticVal += staticV(vec2(uv.x,uv.y+dist))*(maxDist-abs(dist))*1.5;
        }

        staticVal *= bottomStaticOpt;

        if (u_Mirror) uvC.x = 1.0-uvC.x;
        uvC.y = 1.0-uvC.y;
        uvC.y *= 0.5;

        float red 	= texture(s_Texture, mix(uvC, vec2(uv.x + xOffset -0.01*rgbOffsetOpt,y), 0.5)).r+staticVal;
        float green = texture(s_Texture, mix(uvC, vec2(uv.x + xOffset,	                 y), 0.5)).g+staticVal;
        float blue 	= texture(s_Texture, mix(uvC, vec2(uv.x + xOffset +0.01*rgbOffsetOpt,y), 0.5)).b+staticVal;

        vec3 color = vec3(red,green,blue);
        float scanline = sin(uv.y*800.0)*0.04*scalinesOpt;
        color -= scanline;

//        c += texture(s_Texture, uvC);
        c += vec4(color, 1.0);
    }

    return c;
}

vec4 boxB() {
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 c = vec4(0.0, 0.0, 0.0, 0.0);

    vec2 uvC = crtCurveB(fragCoord, 1.0, true, false); 	 // Content Layer
    vec2 uvS = crtCurveB(fragCoord, 1.0, false, false);	 // Screen Layer
    vec2 uvE = crtCurveB(fragCoord, 1.25, false, false); // Enclosure Layer

    // From my shader https://www.shadertoy.com/view/MtBXW3
    //    const float ambient = 0.33;

    // Glass Shine
    vec2 uvSh = crtCurveB(fragCoord, 1.0, false, true);
    c += max(0.0, SHINE - distance(uvSh, vec2(0.5, 1.0))) *
    smoothstep(SMOOTH/2.0, -SMOOTH/2.0, stdRS(uvS + vec2(0., 0.03), 0.0));

    //    // Ambient
    //    c += max(0.0, ambient - 0.5*distance(uvS, vec2(0.5,0.5))) *
    //    smoothstep(SMOOTH, -SMOOTH, stdRS(uvS, 0.0));

    // Enclosure Layer
    uvSh = crtCurveB(fragCoord, 1.25, false, true);
    vec4 b = vec4(0.0, 0.0, 0.0, 0.0);
    for(int i=0; i<12; i++)
    b += (clamp(BEZEL_COL_B + rand(uvSh+float(i))*0.05-0.025, 0.0, 1.0) +
    rand(uvE+1.0+float(i))*0.25 * cos((uvSh.x-0.5)*3.1415*1.5))/12.0;

    // Inner Border
    const float HHW = 0.5 * HEIGHT/WIDTH;

    c += b/3.0*(1.0 + smoothstep(HHW - 0.025, HHW + 0.025, abs(atan(uvS.x-0.5, uvS.y-0.5))/3.1415)
    + smoothstep(HHW + 0.025, HHW - 0.025, abs(atan(uvS.x-0.5, 0.5-uvS.y))/3.1415))*
    smoothstep(-SMOOTH, SMOOTH, stdRS(uvS, 0.0)) *
    smoothstep(SMOOTH, -SMOOTH, stdRS(uvE, 0.05));

    // Inner Border Shine
    c += (b - 0.4)*
    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.505), vec2(WIDTH, HEIGHT) + 0.05, 0.05)) *
    smoothstep(SMOOTH*2.0, -SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.495), vec2(WIDTH, HEIGHT) + 0.05, 0.05));

    // Outer Border
    c += b *
    smoothstep(-SMOOTH, SMOOTH, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.05, 0.05)) *
    smoothstep(SMOOTH, -SMOOTH, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

    // Outer Border Shine
    c += (b - 0.4)*
    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.495), vec2(WIDTH, HEIGHT) + 0.15, 0.05)) *
    smoothstep(SMOOTH*2.0, -SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.505), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

//    // Table and room
//    c += max(0.0, (1.0 - 2.0* fragCoord.y/iResolution.y)) * vec4(1.0, 1.0, 1.0, 0.0) *
//    smoothstep(-0.25, 0.25, roundSquare(uvC - vec2(0.5, -0.2), vec2(WIDTH+0.25, HEIGHT-0.15), 0.1)) *
//    smoothstep(-SMOOTH*2.0, SMOOTH*2.0, roundSquare(uvE-vec2(0.5, 0.5), vec2(WIDTH, HEIGHT) + 0.15, 0.05));

    if (uvC.x > 0.0 && uvC.x < 1.0 && uvC.y > 0.0 && uvC.y < 1.0) {
        vec2 uv = v_texCoord; //fragCoord.xy/u_TexSize.xy;

        float jerkOffset = (1.0-step(snoise(vec2(u_Time*0.33*1.3,5.0)),0.8))*0.05;

        float fuzzOffset = snoise(vec2(u_Time*0.33*15.0,uv.y*80.0))*0.003;
        float largeFuzzOffset = snoise(vec2(u_Time*0.33*1.0,uv.y*25.0))*0.004;

        float vertMovementOn = (1.0-step(snoise(vec2(u_Time*0.33*0.2,8.0)),0.4))*vertMovementOpt;
        float vertJerk = (1.0-step(snoise(vec2(u_Time*0.33*1.5,5.0)),0.6))*vertJerkOpt;
        float vertJerk2 = (1.0-step(snoise(vec2(u_Time*0.33*5.5,5.0)),0.2))*vertJerkOpt;
        float yOffset = abs(sin(u_Time*0.33)*4.0)*vertMovementOn+vertJerk*vertJerk2*0.3;
        float y = mod(uv.y+yOffset,1.0);

        float staticVal = 0.0;
        float xOffset = (fuzzOffset + largeFuzzOffset) * horzFuzzOpt;

        for (float y = -1.0; y <= 1.0; y += 1.0) {
            float maxDist = 5.0/200.0;
            float dist = y/200.0;
            staticVal += staticV(vec2(uv.x,uv.y+dist))*(maxDist-abs(dist))*1.5;
        }

        staticVal *= bottomStaticOpt;

        if (u_Mirror) uvC.x = 1.0-uvC.x;
        uvC.y = 1.0-uvC.y;
        uvC.y = 0.5 + uvC.y * 0.5;

        float red 	= texture(s_Texture, mix(uvC, vec2(uv.x + xOffset -0.01*rgbOffsetOpt,y), 0.5)).r+staticVal;
        float green = texture(s_Texture, mix(uvC, vec2(uv.x + xOffset,	                 y), 0.5)).g+staticVal;
        float blue 	= texture(s_Texture, mix(uvC, vec2(uv.x + xOffset +0.01*rgbOffsetOpt,y), 0.5)).b+staticVal;

        vec3 color = vec3(red,green,blue);
        float scanline = sin(uv.y*800.0)*0.04*scalinesOpt;
        color -= scanline;

//        c += texture(s_Texture, uvC);
        c += vec4(color, 1.0);
    }

    return c;
}

void main() {
    outColor = boxA() + boxB();
}