#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/RHI/Descriptor.h"

namespace rhi {
	class VulkanDevice;

	struct VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
		VkDescriptorSetLayout layout;
	};

	struct VulkanDescriptorSet : public DescriptorSet
	{
		VkDescriptorSet descriptor_set;
	};

	class VulkanDescriptorAllocator : public DescriptorAllocator
	{
		friend class VulkanDescriptorWriter;
	public:
		struct PoolSizes {
			std::vector<std::pair<VkDescriptorType, float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		VulkanDescriptorAllocator(VulkanDevice* in_device);
		virtual ~VulkanDescriptorAllocator();
		virtual void ResetPools() override;
		virtual bool Allocate(DescriptorSet* set, DescriptorSetLayout* layout) override;

		virtual void Shutdown() override;
	private:
		VulkanDevice* device_;
		VkDescriptorPool GrabPool();

		VkDescriptorPool current_pool_{ VK_NULL_HANDLE };
		PoolSizes descriptor_sizes_;
		std::vector<VkDescriptorPool> used_pools_;
		std::vector<VkDescriptorPool> free_pools_;
	};

	class VulkanDescriptorSetLayoutCache : public DescriptorSetLayoutCache
	{
	public:
		VulkanDescriptorSetLayoutCache(VulkanDevice* in_device)
			:device_(in_device) {};
		virtual ~VulkanDescriptorSetLayoutCache();
		virtual void Shutdown() override;
		virtual DescriptorSetLayout* CreateDescriptorLayout(DescriptorLayoutDesc* desc) override;
	private:
		VulkanDevice* device_;
	};

	class VulkanDescriptorWriter : public DescriptorWriter
	{
	public:
		VulkanDescriptorWriter(VulkanDescriptorAllocator* allocator)
			:alloc_(allocator)
		{
			writes_.reserve(10);
		};


		virtual DescriptorWriter& WriteBuffer(uint32_t binding, rhi::RHIBuffer* buffer, DescriptorType type) override;
		virtual DescriptorWriter& WriteImage(uint32_t binding, rhi::RHITexture2D* image, DescriptorType type) override { return *this; };

		virtual bool Build(DescriptorSet* set, DescriptorSetLayout* layout) override;
		virtual void OverWrite(DescriptorSet* set) override;
	private:
		std::vector<VkWriteDescriptorSet> writes_;

		VulkanDescriptorAllocator* alloc_;
	};
}

