#pragma once
#include "Texture.hpp"

//规定  方法必须保证线程安全
class ITraceMethod
{
public:
    virtual void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) = 0;
    virtual ~ITraceMethod() {}
};

//规定 方法必须保证线程安全
class ILoadMethod
{
public:
    virtual void load() = 0;
    virtual ~ILoadMethod() {}
};

class ITracer // Tracer Method 调度器
{
public:
    virtual void render(ITraceMethod &method) = 0;
    virtual TextureID getTraceOutputTextureID() = 0;
    virtual void resetSamples() = 0;
    virtual void resize(int newWidth, int newHeight) = 0;
    virtual void waitForCompletion() = 0;
    virtual ~ITracer() {}
};

//规定 UpLoader 接口
// 将会写入Context的数据
// 异步加载器和同步加载器在行为上相同的点是什么?
class IUpLoader // Loader Method 调度器
{
public:
    virtual void upload(ILoadMethod &method) = 0;
    virtual void waitForCompletion() = 0;
    // WIP
    virtual ~IUpLoader() {}
};

class IRenderPipeline
{
public:
    virtual ~IRenderPipeline() {}
    virtual ILoadMethod *getLoadMethod() = 0;
    virtual ITraceMethod *getTraceMethod() = 0;
};

template <typename ContextType, typename LoadMethodType, typename TraceMethodType>
class RenderPipeline : public IRenderPipeline
{
private:
    std::unique_ptr<LoadMethodType> loadMethod;
    std::unique_ptr<TraceMethodType> traceMethod;
    std::unique_ptr<ContextType> context;

public:
    RenderPipeline(ContextType &&_context)
    {
        context = std::make_unique<ContextType>(std::move(_context));
        loadMethod = std::make_unique<LoadMethodType>(*context);
        traceMethod = std::make_unique<TraceMethodType>(*context);
    }
    ILoadMethod *getLoadMethod() override
    {
        return loadMethod.get();
    }
    ITraceMethod *getTraceMethod() override
    {
        return traceMethod.get();
    }
};
