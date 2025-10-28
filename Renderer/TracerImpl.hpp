#pragma once
#include "RenderInterfaces.hpp"

// WIP
class TracerAsync : public ITracer
{
    Texture2D traceInput;
    Texture2D traceOutput;

    int currentSampleCount = 1;

public:
    TracerAsync(int width, int height)
    {
        // Initialize traceInput and traceOutput textures
        traceInput.generate(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
        traceOutput.generate(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    }

    void render(ITraceMethod &method)
    {
        std::swap(traceInput, traceOutput);
        method.trace(traceInput, traceOutput, currentSampleCount++);
    }

    TextureID getTraceOutputTextureID()
    {
        // Return the texture ID for the trace output
        return traceOutput.ID;
    }

    // WIP
    void resetSamples()
    {
        // Reset sample count or any other state
        traceInput.setData(NULL);  // Clear input texture
        traceOutput.setData(NULL); // Clear output texture
        currentSampleCount = 1;
    }

    void waitForCompletion()
    {
        // Wait for any asynchronous operations to complete
    }

    void resize(int newWidth, int newHeight)
    {
        // Resize internal buffers or textures
        traceInput.resize(newWidth, newHeight);
        traceOutput.resize(newWidth, newHeight);
    }
};