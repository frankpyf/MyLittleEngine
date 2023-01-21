project "Runtime"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"
   
   targetdir ("bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   pchheader "mlepch.h"
   pchsource "src/mlepch.cpp"

   files 
   { 
      "src/**.h", "src/**.cpp", "src/**.hpp" ,
      
      "../vendor/stb_image/**.h",
      "../vendor/stb_image/**.cpp",
      "../vendor/glm/glm/**.hpp",
      "../vendor/glm/glm/**.inl",

      "../vendor/ImGuizmo/ImGuizmo.h",
      "../vendor/ImGuizmo/ImGuizmo.cpp"
   }
   
   includedirs
   {
      "src",
      "../vendor/imgui",
      "../vendor/stb_image",
      "../vendor/vma",

      "%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.glfw}",
      "%{IncludeDir.glm}",
      "%{IncludeDir.spdlog}",
      "%{IncludeDir.ImGuizmo}",
      "%{IncludeDir.entt}"
   }

   links
   {
      "ImGui",
      "GLFW",

      "%{Library.Vulkan}",
   }

   filter "files:../vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

   filter "system:windows"
      systemversion "latest"
      defines { "MLE_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "MLE_DEBUG" }
      runtime "Debug"
      symbols "on"

   filter "configurations:Release"
      defines { "MLE_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      defines { "MLE_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"