#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
/********************视口大小*****************************************/
uniform int width = 1600;
uniform int height = 900;
vec2 noiseScale = vec2(width / 16.0, height / 16.0);

// texture samplers
uniform sampler2D outputLayer0;
uniform sampler2D outputLayer1;
uniform sampler2D texNoise;

void main()
{
    vec4 layer0 = texture(outputLayer0, TexCoord);
    vec4 layer1 = texture(outputLayer1, TexCoord);

    FragColor = vec4(layer1.rgb * layer1.a + layer0.rgb * (1.0 - layer1.a), 1.0);
}