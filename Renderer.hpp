#include "GLShader/Shader.hpp"
#include "GLShader/Pass.hpp"

/// @brief 生成一个程序化的棋盘格纹理
/// @return 成功则返回纹理ID，失败则返回0
// 测试Renderer是否正常工作
inline GLuint GenerateProceduralTexture()
{
    const int textureWidth = 64;
    const int textureHeight = 64;
    // 棋盘格的方块大小
    const int tileSize = 8;

    // 分配内存来存储像素数据
    // 3个字节(RGB) per pixel
    unsigned char *data = new unsigned char[textureWidth * textureHeight * 3];

    // 遍历每个像素，生成棋盘格颜色
    for (int y = 0; y < textureHeight; ++y)
    {
        for (int x = 0; x < textureWidth; ++x)
        {
            // 判断当前像素是哪个颜色
            unsigned char color1_r = 255; // 红色
            unsigned char color1_g = 255;
            unsigned char color1_b = 255;
            unsigned char color2_r = 0; // 黑色
            unsigned char color2_g = 0;
            unsigned char color2_b = 0;

            bool isColor1 = (((x / tileSize) % 2) == 0) ^ (((y / tileSize) % 2) == 0);

            // 获取当前像素在数组中的索引
            int index = (y * textureWidth + x) * 3;

            if (isColor1)
            {
                data[index + 0] = color1_r;
                data[index + 1] = color1_g;
                data[index + 2] = color1_b;
            }
            else
            {
                data[index + 0] = color2_r;
                data[index + 1] = color2_g;
                data[index + 2] = color2_b;
            }
        }
    }

    // 1. 创建 OpenGL 纹理对象
    GLuint textureID;
    glGenTextures(1, &textureID);
    if (textureID == 0)
    {
        std::cerr << "Failed to create OpenGL texture object." << std::endl;
        delete[] data;
        return 0;
    }

    // 2. 绑定纹理
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 3. 将像素数据上传到纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // 4. 设置纹理过滤和环绕模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 5. 释放本地像素数据内存
    delete[] data;

    // 6. 返回纹理ID
    return textureID;
}

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture screenTexture;

public:
    ScreenPass screenPass = ScreenPass(width, height, "GLSL/screenQuad.vs", "GLSL/texture.fs");
    void Render()
    {
        auto TexID = GenerateProceduralTexture();
        screenPass.render(TexID);
    }
};