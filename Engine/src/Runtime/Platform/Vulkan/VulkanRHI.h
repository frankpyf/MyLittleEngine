#pragma once
#include "Runtime/Function/RHI.h"
#include <vulkan/vulkan.h>

namespace engine {
    class VulkanPipeline;
    class VulkanDevice;
    class VulkanViewport;

    class VulkanRHI : public RHI {
    public:
        VulkanRHI();
        virtual void Init() override;
        virtual void Shutdown() override;

        void RHITick(float delta_time) override;
        void RHIBlockUntilGPUIdle() override;

        void GetExtensionsAndLayers();

        inline VkInstance GetInstance() const { return instance_; };
        inline VulkanDevice* GetDevice() { return device_; };
        inline VulkanViewport* GetViewport() { return viewport_; };
    private:
        
        
    protected:
        VkInstance instance_;
        std::vector<const char*> instance_extensions_;
        std::vector<const char*> instance_layers_;

        VulkanDevice* device_;

        VulkanViewport* viewport_;

        void CreateInstance();
        void SelectAndInitDevice();
    };
}