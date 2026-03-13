# Install script for directory: /Users/ahmadrubaii/Engine-Core/vendor/assimp/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/ahmadrubaii/Engine-Core/emsdk/upstream/emscripten/cache/sysroot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp6.0.4-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/ahmadrubaii/Engine-Core/build-web/vendor/assimp/lib/libassimp.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/anim.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/aabb.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ai_assert.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/camera.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/color4.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/color4.inl"
    "/Users/ahmadrubaii/Engine-Core/build-web/vendor/assimp/code/../include/assimp/config.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ColladaMetaData.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/commonMetaData.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/defs.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/cfileio.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/light.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/material.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/material.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/matrix3x3.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/matrix3x3.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/matrix4x4.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/matrix4x4.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/mesh.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ObjMaterial.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/pbrmaterial.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/GltfMaterial.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/postprocess.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/quaternion.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/quaternion.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/scene.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/metadata.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/texture.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/types.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/vector2.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/vector2.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/vector3.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/vector3.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/version.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/cimport.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/AssertHandler.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/importerdesc.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Importer.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/DefaultLogger.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ProgressHandler.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/IOStream.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/IOSystem.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Logger.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/LogStream.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/NullLogger.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/cexport.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Exporter.hpp"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/DefaultIOStream.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/DefaultIOSystem.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ZipArchiveIOSystem.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SceneCombiner.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/fast_atof.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/qnan.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/BaseImporter.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Hash.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/MemoryIOWrapper.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ParsingUtils.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/StreamReader.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/StreamWriter.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/StringComparison.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/StringUtils.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SGSpatialSort.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/GenericProperty.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SpatialSort.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SkeletonMeshBuilder.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SmallVector.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SmoothingGroups.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/SmoothingGroups.inl"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/StandardShapes.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/RemoveComments.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Subdivision.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Vertex.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/LineSplitter.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/TinyFormatter.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Profiler.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/LogAux.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Bitmap.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/XMLTools.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/IOStreamBuffer.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/CreateAnimMesh.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/XmlParser.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/BlobIOSystem.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/MathFunctions.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Exceptional.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/ByteSwapper.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Base64.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Compiler/pushpack1.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Compiler/poppack1.h"
    "/Users/ahmadrubaii/Engine-Core/vendor/assimp/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/ahmadrubaii/Engine-Core/build-web/vendor/assimp/code/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
