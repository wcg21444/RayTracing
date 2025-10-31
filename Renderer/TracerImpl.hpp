#pragma once
#include "RenderInterfaces.hpp"

// WIP
class TracerAsync : public ITracer
{
    Texture2D traceInput;
    Texture2D traceOutput;
    int currentSampleCount = 1;
public:
    TracerAsync(int width, int height);
    void render(ITraceMethod &method) override;
    TextureID getTraceOutputTextureID() override;
    void resetSamples() override;
    void waitForCompletion() override;
    void resize(int newWidth, int newHeight) override;
};