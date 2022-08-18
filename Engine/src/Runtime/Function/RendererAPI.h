#pragma once

namespace engine
{
    class RendererAPI{
    public:
        virtual void Init() = 0;
        virtual void Shutdown() = 0;
    };
} // namespace engine
