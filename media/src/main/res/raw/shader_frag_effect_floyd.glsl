#version 300 es
precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_Texture;
uniform vec2 u_TexSize;
uniform bool u_Mirror;
uniform float u_Time;

const int lookupSize = 64;
const float errorCarry = 0.06;

float getGrayscale(vec2 coords) {
    vec3 sourcePixel = texture(s_Texture, coords).rgb;
    return length(sourcePixel*vec3(0.2126,0.7152,0.0722));
}

void main() {
    int topGapY = int(u_TexSize.y - v_texCoord.y);
    int cornerGapX = int(u_TexSize.x - v_texCoord.x);
    int cornerGapY = int(u_TexSize.y - v_texCoord.y);
    int cornerThreshhold = ((cornerGapX == 0) || (topGapY == 0)) ? 5 : 4;

    if (cornerGapX+cornerGapY < cornerThreshhold) {
        outColor = vec4(0.0,0.0,0.0,1.0);
    } else if (topGapY < 20) {
        if (topGapY == 19) {
            outColor = vec4(0.0,0.0,0.0,1.0);
        } else {
            outColor = vec4(1.0,1.0,1.0,1.0);
        }
    } else {
        float xError = 0.0;
        for(int xLook=0; xLook<lookupSize; xLook++){
            float grayscale = getGrayscale(v_texCoord + vec2(-lookupSize+xLook,0.0));
            grayscale += xError;
            float bit = grayscale >= 0.5 ? 1.0 : 0.0;
            xError = (grayscale - bit)*errorCarry;
        }

        float yError = 0.0;
        for(int yLook=0; yLook<lookupSize; yLook++){
            float grayscale = getGrayscale(v_texCoord + vec2(0.0,-lookupSize+yLook));
            grayscale += yError;
            float bit = grayscale >= 0.5 ? 1.0 : 0.0;
            yError = (grayscale - bit)*errorCarry;
        }

        float finalGrayscale = getGrayscale(v_texCoord);
        finalGrayscale += xError*0.5 + yError*0.5;
        float finalBit = finalGrayscale >= 0.5 ? 1.0 : 0.0;

        outColor = vec4(finalBit,finalBit,finalBit,1.0);
    }
}