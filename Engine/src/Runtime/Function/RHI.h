#pragma once

namespace engine
{
    class RHI{
    public:
        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void RHITick(float delta_time) = 0;
        virtual void RHIBlockUntilGPUIdle() = 0;

    };
} // namespace engine
