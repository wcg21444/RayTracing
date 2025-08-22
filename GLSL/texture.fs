#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
/********************视口大小*****************************************/
uniform int width = 1600;
uniform int height = 900;
vec2 noiseScale = vec2(width/16.0,height/16.0);

// texture samplers
uniform sampler2D tex_sampler;
uniform sampler2D texNoise;

void main() {
    FragColor = texture(tex_sampler, TexCoord);  
    // FragColor += vec4(texture(texNoise, TexCoord * noiseScale));
    // FragColor = vec4(TexCoord.xy, 1.0f,1.0f);
}