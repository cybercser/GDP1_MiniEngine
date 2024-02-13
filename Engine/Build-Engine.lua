project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
    staticruntime "off"

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    files
    {
        "Source/**.h", 
        "Source/**.cpp",
        "Vendor/glad/src/glad.c",
        "Vendor/stb_image/**.h", 
        "Vendor/stb_image/**.cpp",
        "Vendor/imgui/*.h",
        "Vendor/imgui/*.cpp",
        "Vendor/imgui/backends/imgui_impl_opengl3.h",
        "Vendor/imgui/backends/imgui_impl_opengl3.cpp",
        "Vendor/imgui/backends/imgui_impl_glfw.h",
        "Vendor/imgui/backends/imgui_impl_glfw.cpp",
        "Vendor/imgui/backends/imgui_impl_opengl3_loader.h",
        "Vendor/**.natvis",
    }

    includedirs
    {
        "Source/Runtime",
        "Vendor/assimp/include",
        "Vendor/glad/include",
        "Vendor/glfw/include",
        "Vendor/glm/include",
        "Vendor/imgui",
        "Vendor/imgui/backends",
        "Vendor/spdlog/include",
        "Vendor/json/include",
        "Vendor/stb_image",
    }

    libdirs
    {
        "Vendor/assimp/lib",
        "Vendor/glfw/lib-vc2022",
        "Vendor/spdlog/lib",
    }

    links 
	{ 
		"assimp-vc143-mt",
		"glfw3",
	}

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "GLFW_INCLUDE_NONE",
            "WIN32_LEAN_AND_MEAN"
        }

    filter "configurations:Debug"
        defines { "DEBUG", "GLCORE_ENABLE_ASSERTS" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"
