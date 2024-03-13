project "App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++14"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    files
    {
        "Source/**.h",
        "Source/**.cpp",
    }

    includedirs
    {
        "Source",
        -- Include Engine
        "../Engine/Source/Runtime",
        "../Engine/Source/Vendor",
        "../Engine/Source/Editor",
        "../Engine/%{IncludeDir.glad}",
        "../Engine/%{IncludeDir.glm}",
        "../Engine/%{IncludeDir.imgui}",
        "../Engine/%{IncludeDir.stb_image}",
        "../Engine/%{IncludeDir.assimp}",
        "../Engine/%{IncludeDir.spdlog}",
        "../Engine/%{IncludeDir.json}",
    }

    links
    {
        "Engine"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
        postbuildcommands
        {
            "{COPYFILE} ../Engine/Vendor/assimp/bin/assimp-vc143-mt.dll %{cfg.targetdir}/",
            "{COPYFILE} ../Engine/Vendor/glfw/lib-vc2022/*.dll %{cfg.targetdir}/",
            "{COPY} ./Assets %{cfg.targetdir}/Assets"
        }

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"
        postbuildcommands
        {
            "{COPYFILE} ../Engine/Vendor/assimp/bin/assimp-vc143-mt.dll %{cfg.targetdir}/",
            "{COPYFILE} ../Engine/Vendor/glfw/lib-vc2022/*.dll %{cfg.targetdir}/",
			"{COPY} ./Assets %{cfg.targetdir}/Assets"
        }
