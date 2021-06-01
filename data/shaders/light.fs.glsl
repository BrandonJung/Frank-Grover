#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float night_timer;
uniform int flashlight_on;
uniform int is_night;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main()
{
    bool inLight = false;
    float brightness = 1.f;
    float light = 0.f;
    
    if (is_night == 1)
    {
        if (flashlight_on == 1)
        {
            // center in the center of the window
            float x = texcoord.x * 2.f - 1.f;
            float y = texcoord.y * 2.f - 1.f;
            
            // use implicit equation to determine if  point is within flashlight radius
            if (0.5f - pow(x, 2.f) - pow(y, 2.f) > 0.f)
                inLight = true;
            
            // use 2D Gaussian to get soft border for light
            float A = 1.f;
            float sigma = 0.2f;
            light = A * exp(-(pow(x, 2.f) / (2.f * pow(sigma, 2.f)) + pow(y, 2.f) / (2.f * pow(sigma, 2.f))));
        }
        
        // daytime vs. nighttime
        brightness = (0.5f * sin(3.14f / 2.f * cos(night_timer / 4000.f)) + 0.5f);
    }
    
    if (inLight)
    {
        brightness += light;
        brightness = clamp(brightness, 0.f, 1.f);
        color = texture(screen_texture, texcoord) * brightness;
    }
    else
    {
        brightness -= light;
        brightness = clamp(brightness, 0.f, 1.f);
        color = texture(screen_texture, texcoord) * brightness;
    }
}
