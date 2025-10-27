#pragma once
#include "RenderInterfaces.hpp"

//WIP
class TracerAsync : public ITracer
{
public:
    void render(ITraceMethod &method) 
    {

    }

    TextureID getTraceOutputTextureID() 
    {
        // Return the texture ID for the trace output
        return 0;
    }

    void resetSamples() 
    {
        // Reset sample count or any other state
    }
};