-- WalnutExternal.lua

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["glfw"] = "../vendor/GLFW/include"
IncludeDir["spdlog"] = "../vendor/spdlog/include"
IncludeDir["ImGuizmo"] = "../vendor/ImGuizmo"
IncludeDir["entt"] = "../vendor/entt/single_include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

group "Dependencies"
   include "vendor/imgui"
   include "vendor/glfw"
group ""

group "Core"
include "Engine"
group ""