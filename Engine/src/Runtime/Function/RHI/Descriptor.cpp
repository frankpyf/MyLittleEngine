#include "mlepch.h"
#include "Descriptor.h"

#include "RHI.h"
#include "Runtime/Platform/Vulkan/VulkanDescriptor.h"

namespace rhi {
	bool DescriptorLayoutDesc::operator==(const DescriptorLayoutDesc& other) const 
	{
		if (other.bindings.size() != bindings.size()) {
			return false;
		}
		else {
			//compare each of the bindings is the same. Bindings are sorted so they will match
			for (int i = 0; i < bindings.size(); ++i) {
				if (other.bindings[i].binding != bindings[i].binding) {
					return false;
				}
				if (other.bindings[i].descriptor_type != bindings[i].descriptor_type) {
					return false;
				}
				if (other.bindings[i].descriptor_count != bindings[i].descriptor_count) {
					return false;
				}
				if (other.bindings[i].stage != bindings[i].stage) {
					return false;
				}
			}
			return true;
		}
	}

	size_t DescriptorLayoutDesc::Hash() const 
	{
		using std::size_t;
		using std::hash;

		size_t result = hash<size_t>()(bindings.size());

		for (int i = 0; i < bindings.size(); ++i)
		{
			//pack the binding data into a size_t. Not fully correct but it's ok
			size_t binding_hash = bindings[i].binding | 
								  bindings[i].descriptor_type << 8 |
								  bindings[i].descriptor_count << 16 |
								  bindings[i].stage << 24;

			//shuffle the packed binding data and xor it with the main hash
			result ^= hash<size_t>()(binding_hash);
		}

		return result;
	}

	// -----------------------------------------------------------------------------

	DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::Begin(DescriptorSetLayoutCache* cache)
	{
		DescriptorSetLayoutBuilder builder;
		builder.cache_ = cache;
		return builder;
	};

	DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(uint32_t binding, DescriptorType type, ShaderStage stage_flags, uint8_t count)
	{
		auto& current_binding = current_desc_.bindings.emplace_back();
		current_binding.binding = binding;
		current_binding.descriptor_count = count;
		current_binding.descriptor_type = type;
		current_binding.stage = stage_flags;

		return *this;
	}

	DescriptorSetLayoutRef DescriptorSetLayoutBuilder::Build()
	{
		return cache_->CreateDescriptorLayout(current_desc_);
	}

	DescriptorSetLayoutRef DescriptorSetLayoutBuilder::BuildFromDesc(const DescriptorLayoutDesc& in_desc)
	{
		return cache_->CreateDescriptorLayout(in_desc);
	}
	// -----------------------------------------------------------------------------

	DescriptorWriter& DescriptorWriter::Begin(DescriptorAllocator* allocator)
	{
		switch (RHI::GetAPI())
		{
		case RHI::GfxAPI::None:
			assert(false && "Need to select a RendererAPI!"); break;
		case RHI::GfxAPI::Vulkan:
			VulkanDescriptorAllocator* vk_allocator = static_cast<VulkanDescriptorAllocator*>(allocator);
			static VulkanDescriptorWriter builder(vk_allocator);
			builder.writes_.clear();
			return builder;
		}
	};
}