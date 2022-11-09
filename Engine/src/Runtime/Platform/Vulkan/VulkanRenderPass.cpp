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

namespace Utils {
	VkFormat AttachmentFormatToVulkanFormat(renderer::AttachmentFormat in_format)
	{
		switch (in_format)
		{
		case renderer::AttachmentFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
		// TODO: Switch to FindDepthFormat()
		case renderer::AttachmentFormat::DEPTH24STENCIL8:return VK_FORMAT_D32_SFLOAT;
		default:return (VkFormat)0;
		}
	}

	VkAttachmentLoadOp AttachmentLoadOpToVulkanLoadOp(renderer::LoadOp in_op)
	{
		switch (in_op)
		{
		case renderer::LoadOp::LOAD: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case renderer::LoadOp::CLEAR:return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case renderer::LoadOp::DONT_CARE:return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		assert("No Load Operation specified!");
	}

	VkAttachmentStoreOp AttachmentStoreOpToVulkanLoadOp(renderer::StoreOp in_op)
	{
		switch (in_op)
		{
		case renderer::StoreOp::STORE:return VK_ATTACHMENT_STORE_OP_STORE;
		case renderer::StoreOp::DONT_CARE:return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		assert("No Store Operation specified!");
	}

	VkImageLayout ImageLayoutToVkImageLayout(renderer::ImageLayout in_layout)
	{
		switch (in_layout)
		{
		case renderer::ImageLayout::IMAGE_LAYOUT_UNDEFINED:return VK_IMAGE_LAYOUT_UNDEFINED;
		case renderer::ImageLayout::IMAGE_LAYOUT_GENERAL:return VK_IMAGE_LAYOUT_GENERAL;
		case renderer::ImageLayout::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case renderer::ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case renderer::ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case renderer::ImageLayout::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case renderer::ImageLayout::IMAGE_LAYOUT_PRESENT:return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		assert("No Image Layout specified in the Render Pass Desc!");
	}
}

namespace renderer {
	VulkanRenderTarget::VulkanRenderTarget(rhi::VulkanRHI& in_rhi, RenderPass& pass)
		:RenderTarget(pass.GetRTDesc().width, pass.GetRTDesc().height, pass.GetRTDesc().clear_value),
		rhi_(in_rhi)
	{
		CreateFramebuffer(pass);
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		DestroyFramebuffer();
	}

	void VulkanRenderTarget::CreateFramebuffer(RenderPass& pass)
	{
		std::vector<VkImageView> attachments{};
		for (auto attachment : pass.GetRTDesc().attachments)
		{
			attachments.emplace_back((VkImageView)attachment);
		}
		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.flags = 0U;
		framebuffer_create_info.renderPass = (VkRenderPass)pass.GetHandle();
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

	VulkanRenderPass::VulkanRenderPass(rhi::VulkanRHI& in_rhi, const char* render_pass_name, const RenderPassDesc& desc,
		EXEC_FUNC exec)
		:RenderPass(render_pass_name, desc.is_for_present, exec), rhi_(in_rhi)
	{
		CreateRenderPass(desc);
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		rhi_.RHIBlockUntilGPUIdle();

		vkDestroyRenderPass(rhi_.GetDevice()->GetDeviceHandle(), render_pass_, nullptr);
	}

	void VulkanRenderPass::CreateRenderPass(RenderPassDesc desc)
	{
		// TODO: Change this vector thing maybe
		// color attachments descriptions
		std::vector<VkAttachmentDescription> attachments{};
		for (auto color_attachment : desc.color_attachments)
		{
			VkAttachmentDescription attachment_description{};
			attachment_description.format = (color_attachment.format == AttachmentFormat::SWAPCHAIN_FORMAT)?
										     rhi_.GetViewport()->GetSwapChain()->GetImageFormat() : Utils::AttachmentFormatToVulkanFormat(color_attachment.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.loadOp = Utils::AttachmentLoadOpToVulkanLoadOp(color_attachment.load_op);
			attachment_description.storeOp = Utils::AttachmentStoreOpToVulkanLoadOp(color_attachment.store_op);
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment_description.finalLayout = Utils::ImageLayoutToVkImageLayout(color_attachment.final_layout);
			attachments.emplace_back(attachment_description);
		}

		if (desc.depth_stencil_attachment != nullptr)
		{
			// depth stencil attachment description
			VkAttachmentDescription depth_stencil_attachment{};
			depth_stencil_attachment.format = Utils::AttachmentFormatToVulkanFormat(desc.depth_stencil_attachment->format);
			depth_stencil_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			// depth data Load/Store Op
			depth_stencil_attachment.loadOp = Utils::AttachmentLoadOpToVulkanLoadOp(desc.depth_stencil_attachment->load_op);
			depth_stencil_attachment.storeOp = Utils::AttachmentStoreOpToVulkanLoadOp(desc.depth_stencil_attachment->store_op);
			depth_stencil_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_stencil_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_stencil_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_stencil_attachment.finalLayout = Utils::ImageLayoutToVkImageLayout(desc.depth_stencil_attachment->final_layout);
			attachments.emplace_back(depth_stencil_attachment);
			delete desc.depth_stencil_attachment;
		}

		// subpass description
		std::vector<VkSubpassDescription> subpasses{};
		// keep the pointers to VkAttachmentReference
		// if you wanna know why i do this,
		// i will explain it to you personally
		std::vector<VkAttachmentReference*> input_refs{};
		std::vector<VkAttachmentReference*> output_refs{};
		for (auto subpass : desc.subpasses)
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
				VkAttachmentReference depth_stencil = {};
				depth_stencil.attachment = attachments.size() - 2;
				depth_stencil.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpass_desc.pDepthStencilAttachment = &depth_stencil;
			}

			subpasses.push_back(subpass_desc);
		}

		std::vector<VkSubpassDependency> dependencies{};

		// temp
		VkSubpassDependency dependency = {};
		if (subpasses.size() == 1)
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		else
			dependency.srcSubpass = subpasses.size() - 2;
		dependency.dstSubpass = subpasses.size() - 1;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies.emplace_back(dependency);

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
		MLE_CORE_INFO("Vulkan Render Pass has been created");

		if (is_for_present_)
		{
			SetupIMGUI(subpasses.size() - 1);
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

	void VulkanRenderPass::SetupIMGUI(uint32_t present_subpass_index)
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
		init_info.Subpass = present_subpass_index;
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
}