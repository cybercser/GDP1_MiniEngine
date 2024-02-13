-- premake5.lua
workspace "MiniEngine"
   architecture "x64"
   configurations { "Debug", "Release"}
   startproject "App"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

-- Include directories relative to Engine
IncludeDir = {}
IncludeDir["glad"] = "Vendor/glad/include"
IncludeDir["glfw"] = "Vendor/glfw/include"
IncludeDir["glm"] = "Vendor/glm/include"
IncludeDir["imgui"] = "Vendor/imgui"
IncludeDir["stb_image"] = "vendor/stb_image"
IncludeDir["assimp"] = "Vendor/assimp/include"
IncludeDir["spdlog"] = "Vendor/spdlog/include"
IncludeDir["json"] = "Vendor/json/include"

-- Projects
include "Engine/Build-Engine.lua"
include "App/Build-App.lua"