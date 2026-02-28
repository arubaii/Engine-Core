cmake_minimum_required(VERSION 3.31)
project(Engine_Core)
set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
#find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)


# ======================================================
# =============== EnTT (ECS) SETUP =====================
# ======================================================

include(FetchContent)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.13.0
)

FetchContent_MakeAvailable(entt)


add_library(glad STATIC
        vendor/glad/src/gl.c
)

target_include_directories(glad PUBLIC
        ${CMAKE_SOURCE_DIR}/vendor/glad/include
)


# ======================================================
# =============== IMGUI SETUP ==========================
# ======================================================

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw glad)


# ======================================================
# =============== Project Sources ======================
# ======================================================

file(GLOB_RECURSE SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(Engine_Core
        ${SRC_FILES}
        vendor/stb/stb_image.cpp
)

target_compile_definitions(Engine_Core PRIVATE
        PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)


# ======================================================
# =============== Include Directories ==================
# ======================================================

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/assimp)

target_include_directories(Engine_Core PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
)


# ======================================================
# =============== Link Libraries =======================
# ======================================================

if(APPLE)
    target_link_libraries(Engine_Core PRIVATE
            glfw
            #            GLEW::GLEW
            glad
            imgui
            EnTT::EnTT
            yaml-cpp
            assimp
            m
    )
else()
    target_link_libraries(Engine_Core PRIVATE
            glfw
            #            GLEW::GLEW
            glad
            imgui
            EnTT::EnTT
            yaml-cpp
            assimp
    )
endif()