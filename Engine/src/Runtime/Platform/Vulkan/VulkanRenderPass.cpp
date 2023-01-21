#include "mlepch.h"
#include "VulkanRHI.h"
#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

#include "Runtime/Core/Base/Application.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

// Emedded font
#include "Runtime/ImGui/Roboto-Regular.embed"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace rhi {
	VulkanRenderTarget::VulkanRenderTarget(rhi::VulkanRHI& in_rhi, const RenderTarget::Descriptor& desc)
		:RenderTarget(desc.width, desc.height, desc.clear_value),
		rhi_(in_rhi)
	{
		CreateFramebuffer(desc);
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		DestroyFramebuffer();
	}

	void VulkanRenderTarget::CreateFramebuffer(const RenderTarget::Descriptor& desc)
	{
		std::vector<VkImageView> attachments{};
		for (auto attachment : desc.attachments)
		{
			attachments.emplace_back(static_cast<VkImageView>(attachment->GetView()));
		}

		if (desc.pass->is_for_present_)
		{
			attachments.emplace_back((VkImageView)rhi_.GetNativeSwapchainImageView());
			width_ = rhi_.GetViewportWidth();
			height_ = rhi_.GetViewportHeight();
		}

		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.flags = 0U;
		framebuffer_create_info.renderPass = (VkRenderPass)desc.pass->GetHandle();
		framebuffer_create_info.attachmentCount = attachments.size();
		framebuffer_create_info.pAttachments = attachments.data();
		framebuffer_create_info.width = width_;
		framebuffer_create_info.height = height_;
		framebuffer_create_info.layers = 1;
		VkResult result = vkCreateFramebuffer(rhi_.GetDevice()->GetDeviceHandle(), &framebuffer_create_info, nullptr, &framebuffer_);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to create framebuffer with error {0}", result);
			throw std::runtime_error("failed to create framebuffer");
		}
	}

	void VulkanRenderTarget::DestroyFramebuffer()
	{
		vkDestroyFramebuffer(rhi_.GetDevice()->GetDeviceHandle(), framebuffer_, nullptr);
	}

	VulkanRenderPass::VulkanRenderPass(rhi::VulkanRHI& in_rhi, const RenderPass::Descriptor& desc)
		:RenderPass(desc.is_for_present), rhi_(in_rhi)
	{
		CreateRenderPass(desc);
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		rhi_.RHIBlockUntilGPUIdle();

		vkDestroyRenderPass(rhi_.GetDevice()->GetDeviceHandle(), render_pass_, nullptr);
	}

	void VulkanRenderPass::CreateRenderPass(RenderPass::Descriptor desc)
	{
		// TODO: Change this vector thing maybe
		// color attachments descriptions
		std::vector<VkAttachmentDescription>	attachments{};

		VkAttachmentReference					depth_reference{};

		for (const auto& attachment : desc.attachments)
		{
			VkAttachmentDescription attachment_description{};
			attachment_description.format = attachment.is_depth ? rhi_.GetDepthFormat() : VulkanUtils::MLEFormatToVkFormat(attachment.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.loadOp = attachment.is_depth? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VulkanUtils::MLEFormatToVkFormat(attachment.load_op);
			attachment_description.storeOp = attachment.is_depth ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VulkanUtils::MLEFormatToVkFormat(attachment.store_op);
			attachment_description.stencilLoadOp = attachment.is_depth ? VulkanUtils::MLEFormatToVkFormat(attachment.load_op) : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = attachment.is_depth ? VulkanUtils::MLEFormatToVkFormat(attachment.store_op) : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment_description.finalLayout = VulkanUtils::ImageLayoutToVkImageLayout(attachment.final_layout);

			if (attachment.is_depth)
			{
				depth_reference.attachment = attachments.size();
				depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			attachments.emplace_back(attachment_description);
		}

		if (desc.is_for_present)
		{
			VkAttachmentDescription& attachment_description = attachments.emplace_back();
			attachment_description.format = rhi_.GetSwapchainImageFormat();
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		// subpass description
		std::vector<VkSubpassDescription> subpasses{};

		std::vector<VkSubpassDependency> dependencies{};

		std::vector<VkAttachmentReference*> input_refs{};
		std::vector<VkAttachmentReference*> output_refs{};
		for (const auto& subpass : desc.subpasses)
		{
			VkSubpassDescription subpass_desc = {};
			subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			if (!subpass.input_attachments.empty())
			{
				VkAttachmentReference* input_attachments = new VkAttachmentReference[subpass.input_attachments.size()]{};
				for (uint32_t index = 0; index < subpass.input_attachments.size(); index++)
				{
					input_attachments[index] = { subpass.input_attachments[index],
												 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
				}
				subpass_desc.inputAttachmentCount = subpass.input_attachments.size();
				subpass_desc.pInputAttachments = input_attachments;
				input_refs.emplace_back(input_attachments);
			}

			VkAttachmentReference* color_attachments = new VkAttachmentReference[subpass.color_attachments.size()]{};
			for (uint32_t index = 0; index < subpass.color_attachments.size(); index++)
			{
				color_attachments[index] = { subpass.color_attachments[index],
											 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			}
			subpass_desc.colorAttachmentCount = subpass.color_attachments.size();
			subpass_desc.pColorAttachments = color_attachments;
			output_refs.emplace_back(color_attachments);

			if (subpass.use_depth_stencil)
			{
				subpass_desc.pDepthStencilAttachment = &depth_reference;
			}

			subpasses.push_back(subpass_desc);

			for (auto dependency : subpass.dependencies)
			{
				auto& current = dependencies.emplace_back();
				current.srcSubpass = dependency == std::numeric_limits<uint32_t>::max()? VK_SUBPASS_EXTERNAL : dependency;
				current.dstSubpass = subpasses.size() - 1;
				current.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				current.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				current.srcAccessMask = 0;
				current.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
		}

		if (desc.is_for_present)
		{
			VkAttachmentReference* colorAttachmentRef = new VkAttachmentReference{};
			colorAttachmentRef->attachment = subpasses.size();
			colorAttachmentRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			output_refs.emplace_back(colorAttachmentRef);

			VkSubpassDescription& subpass = subpasses.emplace_back();
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = colorAttachmentRef;

			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = subpasses.size() == 1 ? VK_SUBPASS_EXTERNAL : subpasses.size() - 2;
			dependency.dstSubpass = subpasses.size() - 1;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderpass_create_info{};
		renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpass_create_info.attachmentCount = attachments.size();
		renderpass_create_info.pAttachments = attachments.data();
		renderpass_create_info.subpassCount = subpasses.size();
		renderpass_create_info.pSubpasses = subpasses.data();
		renderpass_create_info.dependencyCount = dependencies.size();
		renderpass_create_info.pDependencies = dependencies.data();

		VkResult result = vkCreateRenderPass(
			rhi_.GetDevice()->GetDeviceHandle(), &renderpass_create_info, nullptr, &render_pass_);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("failed to create render pass, err:{0}", result);
			throw std::runtime_error("failed to create render pass");
		}
		MLE_CORE_INFO("[vulkan] Render Pass has been created");

		if (desc.is_for_present)
		{
			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
			//io.ConfigViewportsNoAutoMerge = true;
			//io.ConfigViewportsNoTaskBarIcon = true;

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			// Setup Platform/Renderer backends
			engine::Application& app = engine::Application::GetApp();
			GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
			ImGui_ImplGlfw_InitForVulkan(window, true);
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = rhi_.GetVkInstance();
			init_info.PhysicalDevice = rhi_.GetDevice()->GetPhysicalHandle();
			init_info.Device = rhi_.GetDevice()->GetDeviceHandle();
			init_info.QueueFamily = rhi_.GetDevice()->GetGfxQueue()->GetFamilyIndex();
			init_info.Queue = rhi_.GetDevice()->GetGfxQueue()->GetQueueHandle();
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = rhi_.GetDevice()->GetDescriptorPool();
			init_info.Subpass = subpasses.size() - 1;
			init_info.MinImageCount = 2;
			init_info.ImageCount = rhi_.GetViewport()->GetSwapChain()->GetSwapchainImageCount();
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.Allocator = nullptr;
			init_info.CheckVkResultFn = check_vk_result;
			ImGui_ImplVulkan_Init(&init_info, render_pass_);

			// Load default font
			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
			io.FontDefault = robotoFont;

			// Upload Fonts
			{
				VkResult err = vkResetCommandPool(rhi_.GetDevice()->GetDeviceHandle(), rhi_.GetDevice()->GetCommandPool(), 0);
				check_vk_result(err);
				VkCommandBuffer command_buffer = rhi_.GetDevice()->BeginSingleTimeCommands();

				ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

				rhi_.GetDevice()->EndSingleTimeCommands(command_buffer);
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}
		}

		// clean up
		for (auto input : input_refs)
			delete input;
		input_refs.clear();
		for (auto output : output_refs)
			delete output;
		output_refs.clear();
	}

	void VulkanRenderPass::SetupDependency(uint32_t src_index, uint32_t dst_index)
	{
		VkSubpassDependency dependency;
		dependency.srcSubpass = src_index;
		dependency.dstSubpass = dst_index;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask =
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}
}