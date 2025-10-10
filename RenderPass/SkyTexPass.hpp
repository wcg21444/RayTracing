
#pragma once
#include "Pass.hpp"
#include "Texture.hpp"
class Camera;
class CubemapParameters;

class SkyTexPass : public Pass
{
    TextureCube skyCubemapTex;

private:
    void initializeGLResources();
    void cleanUpGLResources();

public:
    int cubemapSize;
    std::unique_ptr<CubemapParameters> cubemapParam;

public:
    SkyTexPass(std::string _vs_path, std::string _fs_path, /*  std::string _gs_path, */ int _cubemapSize);
    ~SkyTexPass();
    void contextSetup() override;

    void resize(int _width, int _height) override;

    void render(const glm::vec3 & camPos);
    unsigned int getCubemap();
};
