cmake_minimum_required(VERSION 3.20)
project(Fortify LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORTIFY_ENABLE_VALIDATION "Enable Vulkan validation layers" ON)
option(FORTIFY_SHADER_HOT_RELOAD "Enable shader hot reload" ON)

find_package(Vulkan REQUIRED)
add_subdirectory(lib/glfw)

file(GLOB_RECURSE ENGINE_SRC
    Engine/Core/Source/*.cpp
    Engine/Graphics/Source/*.cpp
    Engine/Utility/*.cpp
)

file(GLOB_RECURSE ENGINE_HEADERS
    Engine/Core/Headers/*.h
    Engine/Graphics/Headers/*.h
    Engine/Utility/*.h
)

file(GLOB IMGUI_SRC
    lib/imgui/*.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
    lib/ImGuizmo/*.cpp
    lib/ImFilebrowser/*.cpp
)

add_library(FortifyEngine STATIC ${ENGINE_SRC} ${ENGINE_HEADERS} ${IMGUI_SRC})

target_include_directories(FortifyEngine PUBLIC
    Engine/Core/Headers
    Engine/Graphics/Headers
    Engine/Utility
    lib/imgui
    lib/imgui/backends
    lib/ImGuizmo
    lib/ImFilebrowser
    lib/tinyobj
    lib/stb-master
    lib/VulkanMemoryAllocator/include
    lib/glm
)

target_link_libraries(FortifyEngine PUBLIC Vulkan::Vulkan glfw)

if (WIN32)
    target_compile_definitions(FortifyEngine PUBLIC VK_USE_PLATFORM_WIN32_KHR)
elseif (UNIX)
    target_compile_definitions(FortifyEngine PUBLIC VK_USE_PLATFORM_XCB_KHR)
endif()

if (FORTIFY_ENABLE_VALIDATION)
    target_compile_definitions(FortifyEngine PUBLIC VK_ENABLE_VALIDATION)
endif()

include(cmake/Shaders.cmake)
fortify_compile_shaders(FortifyEngine)

add_executable(FortifyApp App/main.cpp)
target_link_libraries(FortifyApp PRIVATE FortifyEngine)

set_target_properties(FortifyApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

function(assign_source_groups prefix)
    foreach(file ${ARGN})
        if(EXISTS ${file})
            file(RELATIVE_PATH rel_path ${CMAKE_SOURCE_DIR} ${file})
            get_filename_component(dir ${rel_path} PATH)
            string(REPLACE "/" "\\" group_name ${dir})
            source_group("${prefix}\\${group_name}" FILES ${file})
        endif()
    endforeach()
endfunction()

assign_source_groups("Engine" ${ENGINE_SRC} ${ENGINE_HEADERS})
assign_source_groups("ImGui" ${IMGUI_SRC})
assign_source_groups("App" "${CMAKE_SOURCE_DIR}/App/main.cpp")
