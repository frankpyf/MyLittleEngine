#pragma once
#include "Enum.h"

namespace rhi {
    struct RHIBuffer;
    class RHITexture2D;

    struct DescriptorSet {};
    struct DescriptorSetLayout {};

    typedef struct DescriptorBinding {
        uint8_t         binding;
        DescriptorType  descriptor_type;
        ShaderStage     stage;
        uint8_t         descriptor_count = 1;
    } DescriptorBinding;

    typedef struct DescriptorLayoutDesc {
        std::vector<DescriptorBinding> bindings;

        bool operator==(const DescriptorLayoutDesc& other) const;

        size_t Hash() const;
    } DescriptorLayoutDesc;

    class DescriptorAllocator
    {
    public:
        virtual ~DescriptorAllocator() = default;

        virtual void ResetPools() {};
        virtual bool Allocate(DescriptorSet* set, DescriptorSetLayout* layout) = 0;

        virtual void Shutdown() {};
    };

    class DescriptorSetLayoutCache
    {
    public:
        virtual void Shutdown() {};

        virtual DescriptorSetLayout* CreateDescriptorLayout(DescriptorLayoutDesc* desc) = 0;
    private:
        struct DescriptorLayoutHash {

            std::size_t operator()(const DescriptorLayoutDesc& desc) const {
                return desc.Hash();
            }
        };
    protected:
        std::unordered_map<DescriptorLayoutDesc, DescriptorSetLayout*, DescriptorLayoutHash> layout_cache_;
    };

    class DescriptorSetLayoutBuilder
    {
    public:
        static DescriptorSetLayoutBuilder Begin(DescriptorSetLayoutCache* cache);

        DescriptorSetLayoutBuilder& AddBinding(uint32_t binding, DescriptorType type, ShaderStage stage_flags, uint8_t count = 1);
           
        DescriptorSetLayout* Build();
    private:
        DescriptorLayoutDesc current_desc_;
        DescriptorSetLayoutCache* cache_;
    };

    class DescriptorWriter
    {
    public:
        static DescriptorWriter& Begin(DescriptorAllocator* allocator);

        virtual DescriptorWriter& WriteBuffer(uint32_t binding, rhi::RHIBuffer* buffer, DescriptorType type) = 0;
        virtual DescriptorWriter& WriteImage(uint32_t binding, rhi::RHITexture2D* image, DescriptorType type) = 0;

        virtual bool Build(DescriptorSet* set, DescriptorSetLayout* layout) = 0;
        virtual void OverWrite(DescriptorSet* set) = 0;
    private:

        DescriptorAllocator* alloc_;
    };
}

