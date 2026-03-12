cmake_minimum_required(VERSION 3.31)
project(Engine_Core)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

set(CMAKE_CXX_STANDARD 23)

# ======================================================
# =============== Find Packages (Native Only) ==========
# ======================================================

if(NOT EMSCRIPTEN)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
endif()

# ======================================================
# =============== EnTT + GLM (Emscripten) ==============
# ======================================================

include(FetchContent)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.13.0
)

if(EMSCRIPTEN)
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(entt glm)
else()
    FetchContent_MakeAvailable(entt)
endif()

# ======================================================
# =============== GLAD (Native Only) ===================
# ======================================================

if(NOT EMSCRIPTEN)
    add_library(glad STATIC
            vendor/glad/src/gl.c
    )
    target_include_directories(glad PUBLIC
            ${CMAKE_SOURCE_DIR}/vendor/glad/include
    )
endif()

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

if(NOT EMSCRIPTEN)
    target_link_libraries(imgui PUBLIC glfw glad)
endif()

# ======================================================
# =============== Assimp Setup =========================
# ======================================================

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

if(EMSCRIPTEN)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER  ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER  ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/assimp)

# ======================================================
# =============== Project Sources ======================
# ======================================================

file(GLOB_RECURSE SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

list(APPEND SRC_FILES
        ${CMAKE_SOURCE_DIR}/src/utils/WebCursor.cpp
)

add_executable(Engine_Core
        ${SRC_FILES}
        vendor/stb/stb_image.cpp
)

target_compile_definitions(Engine_Core PRIVATE
        PROJECT_DIR="${CMAKE_SOURCE_DIR}"
)

if(EMSCRIPTEN)
    target_compile_definitions(Engine_Core PRIVATE __EMSCRIPTEN__)
endif()

# ======================================================
# =============== Include Directories ==================
# ======================================================

target_include_directories(Engine_Core PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
)

if(NOT EMSCRIPTEN)
    target_include_directories(Engine_Core PRIVATE
            ${CMAKE_SOURCE_DIR}/vendor/glad/include
    )
endif()

# ======================================================
# =============== Link Libraries =======================
# ======================================================

if(EMSCRIPTEN)
    target_compile_options(Engine_Core PRIVATE
            "-fexceptions"
    )
    target_link_options(Engine_Core PRIVATE
            "-fexceptions"
            "-sUSE_GLFW=3"
            "-sFULL_ES3=1"
            "-sUSE_WEBGL2=1"
            "-sWASM=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sFORCE_FILESYSTEM=1"
            "-sASSERTIONS=2"
            "-sSTACK_OVERFLOW_CHECK=2"
            "-sNO_DISABLE_EXCEPTION_CATCHING"
            "-sMAXIMUM_MEMORY=512mb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@assets"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/shaders@shaders"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/config@config"
            "--shell-file"   "${CMAKE_SOURCE_DIR}/shell.html"
    )

    target_link_libraries(Engine_Core PRIVATE
            imgui
            EnTT::EnTT
            glm::glm
            yaml-cpp
            assimp
    )

elseif(APPLE)
    target_link_libraries(Engine_Core PRIVATE
            glfw
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
            glad
            imgui
            EnTT::EnTT
            yaml-cpp
            assimp
    )
endif()