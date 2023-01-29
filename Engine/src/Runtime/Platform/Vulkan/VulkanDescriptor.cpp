#include "mlepch.h"
#include "VulkanDescriptor.h"
#include "VulkanDevice.h"
#include "VulkanResource.h"
#include "VulkanUtils.h"

namespace utils {
	VkDescriptorPool VkCreatePool(VkDevice device, 
		const rhi::VulkanDescriptorAllocator::PoolSizes& pool_sizes, 
		int count,
		VkDescriptorPoolCreateFlags flags)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(pool_sizes.sizes.size());
		for (auto sz : pool_sizes.sizes) {
			sizes.push_back({ sz.first, uint32_t(sz.second * count) });
		}
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = flags;
		pool_info.maxSets = count;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();

		VkDescriptorPool descriptorPool;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

		return descriptorPool;
	}
	VkResult vkCreateDescriptorSetLayoutFromMLEDesc(VkDevice device, rhi::DescriptorLayoutDesc* desc, VkDescriptorSetLayout* layout)
	{
		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = desc->bindings.size();
		std::vector<VkDescriptorSetLayoutBinding> bindings_vk;
		bindings_vk.reserve(desc->bindings.size());

		for (int i = 0; i < desc->bindings.size(); ++i)
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = desc->bindings[i].binding;
			binding.descriptorType = rhi::VulkanUtils::MLEFormatToVkFormat(desc->bindings[i].descriptor_type);
			binding.descriptorCount = desc->bindings[i].descriptor_count;
			binding.stageFlags = rhi::VulkanUtils::MLEFormatToVkFormat(desc->bindings[i].stage);

			bindings_vk.push_back(binding);
		}

		info.pBindings = bindings_vk.data();
		if (vkCreateDescriptorSetLayout(device, &info, nullptr, layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
}

namespace rhi {
	VulkanDescriptorAllocator::VulkanDescriptorAllocator(rhi::VulkanDevice* in_device)
		:device_(in_device)
	{
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
	{
		if (!free_pools_.empty() || !used_pools_.empty())
			Shutdown();
	}

	void VulkanDescriptorAllocator::Shutdown()
	{
		std::for_each(free_pools_.begin(), free_pools_.end(), [=](VkDescriptorPool pool) {
			vkDestroyDescriptorPool(device_->GetDeviceHandle(), pool, nullptr);
			});
		free_pools_.clear();

		std::for_each(used_pools_.begin(), used_pools_.end(), [=](VkDescriptorPool pool) {
			vkDestroyDescriptorPool(device_->GetDeviceHandle(), pool, nullptr);
			});
		used_pools_.clear();
	}

	void VulkanDescriptorAllocator::ResetPools()
	{
		//reset all used pools and add them to the free pools
		for (auto p : used_pools_) {
			vkResetDescriptorPool(device_->GetDeviceHandle(), p, 0);
			free_pools_.push_back(p);
		}

		//clear the used pools, since we've put them all in the free pools
		used_pools_.clear();

		//reset the current pool handle back to null
		current_pool_ = VK_NULL_HANDLE;
	}

	bool VulkanDescriptorAllocator::Allocate(DescriptorSet* set, DescriptorSetLayout* layout)
	{
		//initialize the currentPool handle if it's null
		if (current_pool_ == VK_NULL_HANDLE) {

			current_pool_ = GrabPool();
			used_pools_.push_back(current_pool_);
		}

		VulkanDescriptorSetLayout* layout_vk = (VulkanDescriptorSetLayout*)layout;
		VulkanDescriptorSet* set_vk = (VulkanDescriptorSet*)set;

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;

		allocInfo.pSetLayouts = &layout_vk->layout;
		allocInfo.descriptorPool = current_pool_;
		allocInfo.descriptorSetCount = 1;

		//try to allocate the descriptor set
		VkResult result = vkAllocateDescriptorSets(device_->GetDeviceHandle(), &allocInfo, &set_vk->descriptor_set);
		bool need_reallocate = false;

		switch (result) {
		case VK_SUCCESS:
			//all good, return
			return true;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			//reallocate pool
			need_reallocate = true;
			break;
		default:
			//unrecoverable error
			return false;
		}

		if (need_reallocate) {
			//allocate a new pool and retry
			current_pool_ = GrabPool();
			used_pools_.push_back(current_pool_);
			allocInfo.descriptorPool = current_pool_;

			result = vkAllocateDescriptorSets(device_->GetDeviceHandle(), &allocInfo, &set_vk->descriptor_set);

			//if it still fails then we have big issues
			if (result == VK_SUCCESS) {
				return true;
			}
		}

		return false;
	}

	VkDescriptorPool VulkanDescriptorAllocator::GrabPool()
	{
		//there are reusable pools availible
		if (free_pools_.size() > 0)
		{
			//grab pool from the back of the vector and remove it from there.
			VkDescriptorPool pool = free_pools_.back();
			free_pools_.pop_back();
			return pool;
		}
		else
		{
			//no pools availible, so create a new one
			return utils::VkCreatePool(device_->GetDeviceHandle(), descriptor_sizes_, 1000, 0);
		}
	}

	// ----------------------------------------------------------------------------------
	VulkanDescriptorSetLayoutCache::~VulkanDescriptorSetLayoutCache()
	{
		Shutdown();
	}

	void VulkanDescriptorSetLayoutCache::Shutdown()
	{
		for (auto pair : layout_cache_) {
			VulkanDescriptorSetLayout* layout_vk = (VulkanDescriptorSetLayout*)pair.second;
			vkDestroyDescriptorSetLayout(device_->GetDeviceHandle(), layout_vk->layout, nullptr);
			delete pair.second;
		}
		layout_cache_.clear();
	}

	DescriptorSetLayout* VulkanDescriptorSetLayoutCache::CreateDescriptorLayout(DescriptorLayoutDesc* desc)
	{
		DescriptorLayoutDesc layout_desc;
		layout_desc.bindings.reserve(desc->bindings.size());

		bool is_sorted = true;
		int last_binding = -1;

		//copy from the direct desc struct into our own one
		for (int i = 0; i < desc->bindings.size(); i++)
		{
			layout_desc.bindings.push_back(desc->bindings[i]);

			//check that the bindings are in strict increasing order
			if (desc->bindings[i].binding > last_binding)
			{
				last_binding = desc->bindings[i].binding;
			}
			else
			{
				is_sorted = false;
			}
		}

		//sort the bindings if they aren't in order
		if (!is_sorted)
		{
			std::sort(layout_desc.bindings.begin(), layout_desc.bindings.end(),
				[](DescriptorBinding& a, DescriptorBinding& b)
				{
					return a.binding < b.binding;
				});
		}

		//try to grab from cache
		auto it = layout_cache_.find(layout_desc);
		if (it != layout_cache_.end())
		{
			return (*it).second;
		}
		else
		{
			//create a new one (not found)
			VulkanDescriptorSetLayout* layout = new VulkanDescriptorSetLayout;
			utils::vkCreateDescriptorSetLayoutFromMLEDesc(device_->GetDeviceHandle(), desc, &layout->layout);

			//add to cache
			layout_cache_[layout_desc] = layout;
			return layout;
		}
	}

	// -------------------------------------------------------

	DescriptorWriter& VulkanDescriptorWriter::WriteBuffer(uint32_t binding, rhi::RHIBuffer* buffer, DescriptorType type)
	{
		VulkanBuffer* vk_buffer = static_cast<VulkanBuffer*>(buffer);
		//create the descriptor write
		VkWriteDescriptorSet& newWrite = writes_.emplace_back();

		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;
		newWrite.descriptorCount = 1;
		newWrite.descriptorType = VulkanUtils::MLEFormatToVkFormat(type);
		newWrite.pBufferInfo = &vk_buffer->buffer_info;
		newWrite.dstBinding = binding;
		
		return *this;
	}

	DescriptorWriter& VulkanDescriptorWriter::WriteImage(uint32_t binding, rhi::RHITexture2D* image, DescriptorType type)
	{
		VulkanTexture2D* vk_image = static_cast<VulkanTexture2D*>(image);
		//create the descriptor write
		VkWriteDescriptorSet& newWrite = writes_.emplace_back();

		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;
		newWrite.descriptorCount = 1;
		newWrite.descriptorType = VulkanUtils::MLEFormatToVkFormat(type);
		//newWrite.pBufferInfo = &vk_image->buffer_info;
		newWrite.dstBinding = binding;

		return *this;

	}

	bool VulkanDescriptorWriter::Build(DescriptorSet* set, DescriptorSetLayout* layout)
	{
		if (!alloc_->Allocate(set, layout))
			return false;

		OverWrite(set);
		
		return true;
	}

	void VulkanDescriptorWriter::OverWrite(DescriptorSet* set)
	{
		VulkanDescriptorSet* vk_set = static_cast<VulkanDescriptorSet*>(set);
		//write descriptor
		for (VkWriteDescriptorSet& w : writes_) {
			w.dstSet = vk_set->descriptor_set;
		}

		vkUpdateDescriptorSets(alloc_->device_->GetDeviceHandle(), writes_.size(), writes_.data(), 0, nullptr);
		writes_.clear();
	}
}