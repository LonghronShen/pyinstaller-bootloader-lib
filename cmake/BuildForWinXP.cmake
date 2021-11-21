if(WIN32 OR WIN64)
    add_compile_definitions(
        _VCRT_WIN32_WINNT=_WIN32_WINNT_WINXP 
        _STL_WIN32_WINNT=_WIN32_WINNT_WINXP 
        _WIN32_WINNT=0x0501 
        NTDDI_VERSION=NTDDI_WINXP 
        _USING_V110_SDK71_)
    # Windows XP compatible platform toolset. Must be set before project(),
    # otherwise change of CMAKE_*_TOOLSET will take no effect.
    # We get VS version from the generator name because neither MSVC* nor other
    # variables that describe the compiler aren't available before project().
    message(STATUS "Checking for Visual Studio generator: ${CMAKE_GENERATOR}")
    if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio ([0-9]+)")
        # Note: "v110_xp" is for Visual Studio 11 2012, which is unsupported.
        if(${CMAKE_MATCH_1} EQUAL 12)
            # Visual Studio 12 2013
            set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
            set(CMAKE_VS_PLATFORM_TOOLSET "v120_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
        elseif(${CMAKE_MATCH_1} EQUAL 14)
            # Visual Studio 14 2015
            set(CMAKE_GENERATOR_TOOLSET "v140_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
            set(CMAKE_VS_PLATFORM_TOOLSET "v140_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
        elseif((${CMAKE_MATCH_1} EQUAL 15) OR (${CMAKE_MATCH_1} EQUAL 16))
            # Visual Studio 15 2017 OR Visual Studio 16 2019
            set(CMAKE_GENERATOR_TOOLSET "v141_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
            set(CMAKE_VS_PLATFORM_TOOLSET "v141_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
        elseif(${CMAKE_MATCH_1} EQUAL 11)
            message(FATAL_ERROR "Note: \"v110_xp\" is for Visual Studio 11 2012, which is unsupported.")
        else()
            message(WARNING "Supported Visual Studio: 8, 10, 12 (v120_xp), 14 (v140_xp), 15 (v141_xp), 16 (v141_xp)")
        endif()
    else()
        include(FindWindowsSDK)
        find_package(WindowsSDK REQUIRED)
        set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "7.0" CACHE STRING INTERNAL FORCE)
        set(CMAKE_SYSTEM_VERSION "5.1" CACHE STRING INTERNAL FORCE)
        # if(CMAKE_CL_64)
        #     add_link_options("/SUBSYSTEM:WINDOWS,5.02")
        # else()
        #     add_link_options("/SUBSYSTEM:WINDOWS,5.01")
        # endif()
    endif()
endif()