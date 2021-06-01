#version 330 
// 2D light differences (probably won't use) https://www.shadertoy.com/view/3lScDV
// realistic looking, crayon-y waves from a distance https://www.shadertoy.com/view/wsyfWt
// striped 2d waves (colourful) https://www.shadertoy.com/view/Wl33RN
// single wave with good colour mixing https://www.shadertoy.com/view/MtlBWH

// cartoon waves (main one) https://www.shadertoy.com/view/4sBSzK


in vec2 texcoord;
out vec4 fragColor;

uniform float iTime;
uniform vec3 iResolution;

const float animationSpeed = 0.1;

float sinus1Amplitude = 1.0 / 50.0;
float sinus2Amplitude = 1.0 / 50.0;

float sinus1Lambda = 0.1;
float sinus2Lambda = 0.1;

float sinusYRandomFactor = 0.000002;

float sinus1Movement = 2.0;
float sinus2Movement =  2.0;

float sinus1InnerAmplitude = 1.0 / 64.0;
float sinus2InnerAmplitude = 1.0 / 64.0;

float sinus1InnerLambda = 1.0 / 100.0;
float sinus2InnerLambda = 1.0 / 100.0;

// for best results, use an even numWaves
const int numWaves = 6;

float rand(vec2 co){
    float rnd = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
    if(rnd < 0.35) {
        rnd = 0.35;
    }
    return rnd;
}

float clamped_dist(float y, float waveHeight) {
    return clamp(abs(y - waveHeight) * (30.0), 0.0, 1.0);
}

float dist(float y, float waveHeight) {
    return min(clamped_dist(y, waveHeight), min(clamped_dist(y - 1.0, waveHeight), clamped_dist(y + 1.0, waveHeight)));
}

float min_dist(vec2 uv, float[numWaves] waveHeights) {
   float minSoFar = 1.0;
   for (int i = 0; i < numWaves; i++) {
       minSoFar = min(minSoFar, dist(uv.y, waveHeights[i]));
   }
   return minSoFar - (uv.y * uv.y / 1.1);
}

void main()
{

 float rnd =  rand(vec2(1.0, 1.0));
    float tick = iTime * animationSpeed;
  vec2 uv = texcoord.xy; // / iResolution.xy; // for testing in shadertoy
    
   vec4 whiteColor = vec4(0.3, 0.3, 0.3, 1.0);
    
    vec4 aqua2 = vec4(0.2, 0.4, 0.6, 1.0);
    vec4 darkGreen = vec4(0.0, 0.2, 0.4, 1.0);
    
   sinus1Movement *= rnd;
   sinus2Movement *= rnd;
    
    
    vec4 water = mix(aqua2, darkGreen, (1.0 - uv.y) * 4.0 - 1.5 + sin(iTime + uv.x * 8.0)/6.0);
    
    float waveHeights[numWaves];
    float increment = 1.0 / float(numWaves);
    float offset = 0.0;
    for (int i = 0; i < numWaves; i++) {
        if (i % 2 == 0) {
            waveHeights[i] = mod(mod(tick, 1.0) + sin((tick + uv.x + 0.5) / sinus1Lambda) * sinus1Amplitude + (sin(-tick + (uv.x / sinus1InnerLambda)) * sinus1InnerAmplitude) + offset, 1.0);
        } else {
            waveHeights[i] = mod(mod(tick, 1.0) + sin((tick + uv.x) / sinus2Lambda) * sinus2Amplitude + (sin(-tick + (uv.x / sinus2InnerLambda)) * sinus2InnerAmplitude) + offset, 1.0);
        }
        offset += increment;
    }
    
    if (min_dist(uv, waveHeights) < increment / 2.0) {
        fragColor = mix(water + whiteColor, water, min_dist(uv, waveHeights));
    } else {
        fragColor = water;
    }
}