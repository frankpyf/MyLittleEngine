#pragma once
#include "Runtime/Function/RendererAPI.h"
#include <vulkan/vulkan.h>

namespace engine {
    class VulkanRHI : public RendererAPI {
    public:
        virtual void Init() override;
        virtual void Shutdown() override;
    private:
        void Setup(const char** extensions, uint32_t extensions_count);
        void CreateInstance(const char** extensions, uint32_t extensions_count);
        void PickPhysicalDevice();
        void CreateLogicalDevice();
    private:
        VkInstance       instance_;
        VkPhysicalDevice physical_device_;
        VkDevice         device_;
        VkQueue          graphics_queue_;
        VkAllocationCallbacks* allocator_;
    };
}