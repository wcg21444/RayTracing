#include "TracerImpl.hpp"
#include <utility>

TracerAsync::TracerAsync(int width, int height) {
    traceInput.generate(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    traceOutput.generate(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
}

void TracerAsync::render(ITraceMethod &method) {
    std::swap(traceInput, traceOutput);
    method.trace(traceInput, traceOutput, currentSampleCount++);
}

TextureID TracerAsync::getTraceOutputTextureID() {
    return traceOutput.ID;
}

void TracerAsync::resetSamples() {
    traceInput.setData(NULL);
    traceOutput.setData(NULL);
    currentSampleCount = 1;
}

void TracerAsync::waitForCompletion() {
    // Wait for any asynchronous operations to complete
}

void TracerAsync::resize(int newWidth, int newHeight) {
    traceInput.resize(newWidth, newHeight);
    traceOutput.resize(newWidth, newHeight);
}
