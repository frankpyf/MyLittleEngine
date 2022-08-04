project "Engine"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp", "src/**.hpp" }

   pchheader "mlepch.h"
   pchsource "src/mlepch.cpp"

   includedirs
   {
      "src",
      "../vendor/imgui",
      "../vendor/GLFW/include",
      "../vendor/stb_image",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
   }

   links
   {
       "ImGui",
       "GLFW",

       "%{Library.Vulkan}",
   }

   targetdir ("bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

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