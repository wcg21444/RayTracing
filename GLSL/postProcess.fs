
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

/*****************Screen输入*****************************************************************/
uniform sampler2D screenTex;

/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/*****************toggle设置******************************************************************/
uniform int GammaCorrection;

/*****************效果参数设置******************************************************************/

uniform float gamma;

void main()
{
    FragColor = texture(screenTex, TexCoord);
    // Gamma 矫正
    if (GammaCorrection == 1)
    {
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
    }
}
