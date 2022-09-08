#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/Renderer/Renderer.h"

namespace engine {
    class VulkanRenderer :
        public Renderer
    {
    public:
        virtual void BeginFrame() override;
        virtual void EndFrame() override;
    private:
        virtual void CreateCommandBuffers() override;
        virtual void FreeCommandBuffers() override;


        std::vector<VkCommandBuffer> command_buffers_;
    };
}

