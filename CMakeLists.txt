cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# MSVC runtime library flags are selected by an abstraction
cmake_policy(SET CMP0091 NEW)

project(isle CXX)

include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
include(CMakePushCheckState)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/reccmp.cmake")

set(CMAKE_CXX_STANDARD 20)

set(BUILD_FOR_MODERN_OS FALSE CACHE BOOL "Compile the game to run on a modern OS. Will use Vulkan and SDL instead of DirectX5 and DirectDraw.")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
option(ENABLE_CLANG_TIDY "Enable clang-tidy")
if (ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_BIN NAMES "clang-tidy")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_BIN}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BIN}")
endif()

if (NOT BUILD_FOR_MODERN_OS)
  math(EXPR bits "8 * ${CMAKE_SIZEOF_VOID_P}")
  message(STATUS "Building ${bits}-bit LEGO Island")
  if (NOT bits EQUAL 32 AND NOT BUILD_FOR_MODERN_OS)
    message(WARNING "Only 32-bit executables are supported. Set BUILD_FOR_MODERN_OS to TRUE to enable 64-bit builds.")
  endif()
endif()

set(MSVC_FOR_DECOMP FALSE)
if (MSVC AND ${BUILD_FOR_MODERN_OS})
  # Visual C++ 4.2 -> cl version 10.2.0
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
    set(MSVC_FOR_DECOMP TRUE)
  endif()
endif()

function(add_cxx_warning WARNING)
  if(ISLE_WERROR)
    set(compiler_option "-Werror=${WARNING}")
  else()
    set(compiler_option "-W${WARNING}")
  endif()
  string(MAKE_C_IDENTIFIER "COMPILER_SUPPORTS${compiler_option}" varname)

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${compiler_option} ")
  if(MSVC)
    string(APPEND CMAKE_REQUIRED_FLAGS "/WX")
  else()
    string(APPEND CMAKE_REQUIRED_FLAGS "-Werror")
  endif()
  check_cxx_source_compiles("int main() { return 0; }" ${varname})
  cmake_pop_check_state()

  if(${varname})
    add_compile_options(${compiler_option})
  endif()
endfunction()

message(STATUS "MSVC for decompilation: ${MSVC_FOR_DECOMP}")

option(ISLE_WERROR "Treat warnings as errors" OFF)
option(ISLE_BUILD_APP "Build ISLE.EXE application" ON)
cmake_dependent_option(ISLE_BUILD_CONFIG "Build CONFIG.EXE application" ON "NOT MINGW AND NOT BUILD_FOR_MODERN_OS" OFF)
option(ISLE_USE_SMARTHEAP "Build LEGO1.DLL with SmartHeap" ${MSVC_FOR_DECOMP})
option(ISLE_USE_DX5 "Build with internal DirectX 5 SDK" ON)
cmake_dependent_option(ISLE_USE_DX5 "Build with internal DirectX 5 SDK" ON "NOT BUILD_FOR_MODERN_OS" OFF)
option(ISLE_DECOMP_ASSERT "Assert struct size" ${MSVC_FOR_DECOMP})
cmake_dependent_option(ISLE_USE_DX5_LIBS "Build with internal DirectX 5 SDK Libraries" ON ISLE_USE_DX5 OFF)
option(ISLE_BUILD_LEGO1 "Build LEGO1.DLL library" ON)
option(ISLE_BUILD_BETA10 "Build BETA10.DLL library" OFF)

#TODO(KL): Temporarily disabled
#if(NOT (ISLE_BUILD_LEGO1 OR ISLE_BUILD_BETA10))
#  message(FATAL_ERROR "ISLE_BUILD_LEGO1 AND ISLE_BUILD_BETA10 cannot be both disabled")
#endif()

add_cxx_warning(parentheses)

add_library(DirectX5::DirectX5 INTERFACE IMPORTED)
target_include_directories(DirectX5::DirectX5 INTERFACE "${PROJECT_SOURCE_DIR}/3rdparty/dx5/inc")
if(ISLE_USE_DX5_LIBS)
  target_link_directories(DirectX5::DirectX5 INTERFACE "${PROJECT_SOURCE_DIR}/3rdparty/dx5/lib")
endif()
if (NOT MSVC_FOR_DECOMP)
  configure_file(${PROJECT_SOURCE_DIR}/3rdparty/dx5/bin/d3drm.dll ${CMAKE_BINARY_DIR}/Debug/d3drm.dll COPYONLY)
  configure_file(${PROJECT_SOURCE_DIR}/3rdparty/dx5/bin/d3drm.dll ${CMAKE_BINARY_DIR}/Release/d3drm.dll COPYONLY)
  configure_file(${PROJECT_SOURCE_DIR}/3rdparty/dx5/bin/d3drm.dll ${CMAKE_BINARY_DIR}/RelWithDebInfo/d3drm.dll COPYONLY)
  configure_file(${PROJECT_SOURCE_DIR}/3rdparty/dx5/bin/d3drm.dll ${CMAKE_BINARY_DIR}/MinSizeRel/d3drm.dll COPYONLY)
endif()

add_library(Smacker::Smacker STATIC IMPORTED)
set_property(TARGET Smacker::Smacker PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/3rdparty/smacker/smack.lib")
set_property(TARGET Smacker::Smacker PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rdparty/smacker")

add_library(Vec::Vec INTERFACE IMPORTED)
target_include_directories(Vec::Vec INTERFACE "${PROJECT_SOURCE_DIR}/3rdparty/vec")

add_library(SmartHeap::SmartHeap STATIC IMPORTED)
set_property(TARGET SmartHeap::SmartHeap PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/3rdparty/smartheap/SHLW32MT.LIB")
set_property(TARGET SmartHeap::SmartHeap PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/3rdparty/smartheap")
set_property(TARGET SmartHeap::SmartHeap PROPERTY INTERFACE_COMPILE_OPTIONS "/FI${PROJECT_SOURCE_DIR}/3rdparty/smartheap/SMRTHEAP.HPP")

add_library(inifile-cpp::inifile-cpp INTERFACE IMPORTED)
target_include_directories(inifile-cpp::inifile-cpp INTERFACE "${PROJECT_SOURCE_DIR}/3rdparty/inifile-cpp/include")

if (BUILD_FOR_MODERN_OS)
  # Expects env VULKAN_SDK to be set and SDL2 installed with it
  find_package(Vulkan REQUIRED)

  list(APPEND CMAKE_PREFIX_PATH "$ENV{VULKAN_SDK}/cmake")
  find_package(SDL2 REQUIRED)
endif()

function(add_lego_libraries NAME)
  cmake_parse_arguments(ARG "" "SUFFIX;OUT_TARGETS;DLL_OUTPUT_NAME;DLL_PREFIX;DLL_SUFFIX" "LINK_LIBRARIES" ${ARGN})

  set(list_targets )

  set(SUFFIX "${ARG_SUFFIX}")
  
  file(GLOB_RECURSE tglrlSrc
        "LEGO1/tgl/*.cpp"
        "LEGO1/tgl/*.h")
  add_library(tglrl${ARG_SUFFIX} STATIC ${tglrlSrc})
  list(APPEND list_targets tglrl${ARG_SUFFIX})
  set_property(TARGET tglrl${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "tglrl40$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(tglrl${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(tglrl${ARG_SUFFIX} PRIVATE d3drm)

  file(GLOB_RECURSE realtimeSrc
        "LEGO1/realtime/*.cpp"
        "LEGO1/realtime/*.h")
  add_library(realtime${ARG_SUFFIX} STATIC ${realtimeSrc})
  list(APPEND list_targets realtime${ARG_SUFFIX})
  set_property(TARGET realtime${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "realtime$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(realtime${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(realtime${ARG_SUFFIX} PRIVATE Vec::Vec)

  file(GLOB_RECURSE viewmanagerSrc
        "LEGO1/viewmanager/*.cpp"
        "LEGO1/viewmanager/*.h")
  add_library(viewmanager${ARG_SUFFIX} STATIC ${viewmanagerSrc})
  list(APPEND list_targets viewmanager${ARG_SUFFIX})
  set_property(TARGET viewmanager${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "viewmanager$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(viewmanager${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(viewmanager${ARG_SUFFIX} PRIVATE Vec::Vec)

  #TODO(KL): Make Vulkan version of this for BUILD_FOR_MODERN_OS
  file(GLOB_RECURSE mxdirectxSrc
        "LEGO1/mxdirectx/*.cpp"
        "LEGO1/mxdirectx/*.h")
  add_library(mxdirectx${ARG_SUFFIX} STATIC ${mxdirectxSrc})
  list(APPEND list_targets mxdirectx${ARG_SUFFIX})
  set_property(TARGET mxdirectx${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "MxDirectX$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(mxdirectx${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(mxdirectx${ARG_SUFFIX} PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:ddraw>)

  file(GLOB_RECURSE roiSrc
        "LEGO1/lego/sources/roi/*.cpp"
        "LEGO1/lego/sources/roi/*.h")
  add_library(roi${ARG_SUFFIX} STATIC ${roiSrc})
  list(APPEND list_targets roi${ARG_SUFFIX})
  set_property(TARGET roi${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "roi$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(roi${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(roi${ARG_SUFFIX} PRIVATE viewmanager${ARG_SUFFIX} Vec::Vec)

  file(GLOB_RECURSE geomSrc
        "LEGO1/lego/sources/geom/*.cpp"
        "LEGO1/lego/sources/geom/*.h")
  add_library(geom${ARG_SUFFIX} STATIC ${geomSrc})
  list(APPEND list_targets geom${ARG_SUFFIX})
  set_property(TARGET geom${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "geom$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(geom${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(geom${ARG_SUFFIX} PRIVATE)

  file(GLOB_RECURSE shapeSrc
        "LEGO1/lego/sources/shape/*.cpp"
        "LEGO1/lego/sources/shape/*.h")
  add_library(shape${ARG_SUFFIX} STATIC ${shapeSrc})
  list(APPEND list_targets shape${ARG_SUFFIX})
  set_property(TARGET shape${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "shape$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(shape${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(shape${ARG_SUFFIX} PRIVATE)

  file(GLOB_RECURSE animSrc
        "LEGO1/lego/sources/anim/*.cpp"
        "LEGO1/lego/sources/anim/*.h")
  add_library(anim${ARG_SUFFIX} STATIC ${animSrc})
  list(APPEND list_targets anim${ARG_SUFFIX})
  set_property(TARGET anim${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "anim$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(anim${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources" "${PROJECT_SOURCE_DIR}/util")

  file(GLOB_RECURSE miscSrc
        "LEGO1/lego/sources/misc/*.cpp"
        "LEGO1/lego/sources/misc/*.h")
  add_library(misc${ARG_SUFFIX} STATIC ${miscSrc})
  list(APPEND list_targets misc${ARG_SUFFIX})
  set_property(TARGET misc${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "misc$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(misc${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(misc${ARG_SUFFIX} PRIVATE)

  file(GLOB_RECURSE 3dmanagerSrc
        "LEGO1/lego/sources/3dmanager/*.cpp"
        "LEGO1/lego/sources/3dmanager/*.h")
  add_library(3dmanager${ARG_SUFFIX} STATIC ${3dmanagerSrc})
  list(APPEND list_targets 3dmanager${ARG_SUFFIX})
  set_property(TARGET 3dmanager${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "3dmanager$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(3dmanager${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(3dmanager${ARG_SUFFIX} PRIVATE Vec::Vec)

  file(GLOB_RECURSE omniSrc "LEGO1/omni/*.cpp")
  file(GLOB_RECURSE omniInc "LEGO1/omni/*.h")
  add_library(omni${ARG_SUFFIX} STATIC ${omniSrc} ${omniInc})
  list(APPEND list_targets omni${ARG_SUFFIX})
  set_property(TARGET omni${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "omni$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(omni${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(omni${ARG_SUFFIX} PRIVATE dsound winmm Smacker::Smacker)
  if(MSVC)
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/omni/src" PREFIX "Source Files" FILES ${omniSrc})
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/omni/include" PREFIX "Header Files" FILES ${omniInc})
  else()
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/omni" FILES ${omniSrc} ${omniInc})
  endif()

  file(GLOB_RECURSE legoSrc "LEGO1/lego/legoomni/*.cpp")
  file(GLOB_RECURSE legoInc "LEGO1/lego/legoomni/*.h")
  set(legoAdditionalSrc
    LEGO1/main.cpp
    LEGO1/modeldb/modeldb.cpp
    LEGO1/modeldb/modeldb.h
    LEGO1/define.cpp
    LEGO1/define.h)
  add_library(${NAME} SHARED ${legoSrc} ${legoInc} ${legoAdditionalSrc})
  list(APPEND list_targets ${NAME})
  
  if(MSVC)
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/lego/legoomni/src" PREFIX "Source Files" FILES ${legoSrc})
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/lego/legoomni/include" PREFIX "Header Files" FILES ${legoInc})
  else()
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO1/lego/legoomni" FILES ${omniSrc} ${omniInc})
  endif()

  if (MINGW)
    target_compile_definitions(${NAME} PRIVATE DIRECTINPUT_VERSION=0x0500)
  endif()

  if (MSVC)
    target_sources(${NAME} PRIVATE LEGO1/LegoOmni.def)
  else()
    target_sources(${NAME} PRIVATE LEGO1/LegoOmni.mingw.def)
  endif()

  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/util")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO1")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO1/omni/include")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO1/lego/sources")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO1/lego/legoomni/include")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO1/lego/legoomni/include/actions")

  # Link libraries
  target_link_libraries(${NAME} PRIVATE
    dinput
    misc${ARG_SUFFIX}
    geom${ARG_SUFFIX}
    mxdirectx${ARG_SUFFIX}
    anim${ARG_SUFFIX}
    tglrl${ARG_SUFFIX}
    realtime${ARG_SUFFIX}
    viewmanager${ARG_SUFFIX}
    roi${ARG_SUFFIX}
    3dmanager${ARG_SUFFIX}
    omni${ARG_SUFFIX}
    shape${ARG_SUFFIX}
    Vec::Vec
    dxguid
  )

  foreach(tgt IN LISTS list_targets)
    target_link_libraries(${tgt} PRIVATE ${ARG_LINK_LIBRARIES})
    target_link_libraries(${tgt} PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:DirectX5::DirectX5>)
    target_compile_definitions(${tgt} PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:DIRECTX5_SDK>)
    if (BUILD_FOR_MODERN_OS)
      target_compile_definitions(${tgt} PRIVATE "BUILD_FOR_MODERN_OS")
    endif()
    install(TARGETS ${tgt} DESTINATION .)
  endforeach()

  # Make sure filenames are ALL CAPS
  set_property(TARGET ${NAME} PROPERTY OUTPUT_NAME "${ARG_DLL_OUTPUT_NAME}")
  set_property(TARGET ${NAME} PROPERTY PREFIX "${ARG_DLL_PREFIX}")
  set_property(TARGET ${NAME} PROPERTY SUFFIX "${ARG_DLL_SUFFIX}")

  set(${ARG_OUT_TARGETS} ${list_targets} PARENT_SCOPE)
endfunction()

function(add_lego64_libraries NAME)
  cmake_parse_arguments(ARG "" "SUFFIX;OUT_TARGETS;DLL_OUTPUT_NAME;DLL_PREFIX;DLL_SUFFIX" "LINK_LIBRARIES" ${ARGN})

  set(list_targets )

  set(SUFFIX "${ARG_SUFFIX}")

  file(GLOB_RECURSE mxvulkanSrc
        "LEGO64/mxvulkan/*.cpp"
        "LEGO64/mxvulkan/*.h")
  add_library(mxvulkan STATIC ${mxvulkanSrc})
  list(APPEND list_targets mxvulkan)
  set_property(TARGET mxvulkan PROPERTY ARCHIVE_OUTPUT_NAME "MxVulkan$<$<CONFIG:Debug>:d>")
  target_include_directories(mxvulkan PRIVATE "${PROJECT_SOURCE_DIR}/LEGO64" "${PROJECT_SOURCE_DIR}/util")
  target_include_directories(mxvulkan PRIVATE ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(mxvulkan PRIVATE Vulkan-1)

  file(GLOB_RECURSE realtimeSrc
      "LEGO1/realtime/*.cpp"
      "LEGO1/realtime/*.h")
  add_library(realtime${ARG_SUFFIX} STATIC ${realtimeSrc})
  list(APPEND list_targets realtime${ARG_SUFFIX})
  set_property(TARGET realtime${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "realtime$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(realtime${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO1" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(realtime${ARG_SUFFIX} PRIVATE Vec::Vec)

  file(GLOB_RECURSE omniSrc "LEGO64/omni/*.cpp")
  file(GLOB_RECURSE omniInc "LEGO64/omni/*.h")
  add_library(omni${ARG_SUFFIX} STATIC ${omniSrc} ${omniInc})
  list(APPEND list_targets omni${ARG_SUFFIX})
  set_property(TARGET omni${ARG_SUFFIX} PROPERTY ARCHIVE_OUTPUT_NAME "omni$<$<CONFIG:Debug>:d>${ARG_SUFFIX}")
  target_include_directories(omni${ARG_SUFFIX} PRIVATE "${PROJECT_SOURCE_DIR}/LEGO64/omni/include" "${PROJECT_SOURCE_DIR}/LEGO64" "${PROJECT_SOURCE_DIR}/util")
  target_link_libraries(omni${ARG_SUFFIX} PRIVATE dsound winmm)
  if(MSVC)
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/omni/src" PREFIX "Source Files" FILES ${omniSrc})
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/omni/include" PREFIX "Header Files" FILES ${omniInc})
  else()
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/omni" FILES ${omniSrc} ${omniInc})
  endif()

  file(GLOB_RECURSE legoSrc "LEGO64/lego/legoomni/*.cpp")
  file(GLOB_RECURSE legoInc "LEGO64/lego/legoomni/*.h")
  set(legoAdditionalSrc
    LEGO64/main.cpp
    LEGO64/modeldb/modeldb.cpp
    LEGO64/modeldb/modeldb.h
    LEGO64/define.cpp
    LEGO64/define.h)
  add_library(${NAME} STATIC ${legoSrc} ${legoInc} ${legoAdditionalSrc})
  list(APPEND list_targets ${NAME})
  
  if(MSVC)
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/lego/legoomni/src" PREFIX "Source Files" FILES ${legoSrc})
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/lego/legoomni/include" PREFIX "Header Files" FILES ${legoInc})
  else()
    source_group(TREE "${PROJECT_SOURCE_DIR}/LEGO64/lego/legoomni" FILES ${omniSrc} ${omniInc})
  endif()

  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/util")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO64")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO64/omni/include")
  #target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO64/lego/sources")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO64/lego/legoomni/include")
  target_include_directories(${NAME} PUBLIC "${PROJECT_SOURCE_DIR}/LEGO64/lego/legoomni/include/actions")

  # Link libraries
  target_link_libraries(${NAME} PRIVATE
    realtime${ARG_SUFFIX}
    omni${ARG_SUFFIX}
    Vec::Vec
    mxvulkan
  )

  foreach(tgt IN LISTS list_targets)
    target_link_libraries(${tgt} PRIVATE ${ARG_LINK_LIBRARIES})
      target_compile_definitions(${tgt} PRIVATE "BUILD_FOR_MODERN_OS")
    install(TARGETS ${tgt} DESTINATION .)
  endforeach()

  # Make sure filenames are ALL CAPS
  set_property(TARGET ${NAME} PROPERTY OUTPUT_NAME "${ARG_DLL_OUTPUT_NAME}")
  set_property(TARGET ${NAME} PROPERTY PREFIX "${ARG_DLL_PREFIX}")
  set_property(TARGET ${NAME} PROPERTY SUFFIX "${ARG_DLL_SUFFIX}")

  set(${ARG_OUT_TARGETS} ${list_targets} PARENT_SCOPE)
endfunction()

set(lego1_link_libraries )
if (ISLE_USE_SMARTHEAP)
  list(APPEND lego1_link_libraries SmartHeap::SmartHeap)
endif()

if(ISLE_BUILD_LEGO1)
  if (BUILD_FOR_MODERN_OS)
    add_lego64_libraries(lego1
      LINK_LIBRARIES ${lego1_link_libraries}
      DLL_OUTPUT_NAME "LEGO1"
      DLL_PREFIX ""
      DLL_SUFFIX ".lib"
      OUT_TARGETS lego1_targets
    )
  else()
    add_lego_libraries(lego1
      LINK_LIBRARIES ${lego1_link_libraries}
      DLL_OUTPUT_NAME "LEGO1"
      DLL_PREFIX ""
      DLL_SUFFIX ".DLL"
      OUT_TARGETS lego1_targets
    )
    reccmp_add_target(lego1 ID LEGO1)
  endif()
endif()

if(ISLE_BUILD_BETA10)
  add_lego_libraries(beta10
    SUFFIX "-beta10"
    DLL_OUTPUT_NAME "BETA10"
    DLL_PREFIX ""
    DLL_SUFFIX ".DLL"
    OUT_TARGETS beta10_targets
  )
  reccmp_add_target(beta10 ID BETA10)
  target_compile_definitions(beta10 PRIVATE BETA10)
endif()

if (ISLE_BUILD_APP)
if (BUILD_FOR_MODERN_OS)
  set(isleSrc ISLE/isleapp64.cpp)
  set(isleInc ISLE/isleapp64.h)
else()
  set(isleSrc ISLE/isleapp.cpp)
  set(isleInc ISLE/isleapp.h)
endif()
  set(isleInc ${isleInc} ISLE/res/resource.h)
  set(isleRes ISLE/res/isle.rc)
  source_group(TREE "${PROJECT_SOURCE_DIR}/ISLE" PREFIX "Source Files" FILES ${isleSrc})
  source_group(TREE "${PROJECT_SOURCE_DIR}/ISLE" PREFIX "Header Files" FILES ${isleInc})
  source_group(TREE "${PROJECT_SOURCE_DIR}/ISLE/res" PREFIX "Resource Files" FILES ${isleRes})
  add_executable(isle WIN32 ${isleSrc} ${isleInc} ${isleRes})
  reccmp_add_target(isle ID ISLE)

  target_compile_definitions(isle PRIVATE ISLE_APP)

  if (BUILD_FOR_MODERN_OS)
    target_compile_definitions(isle PRIVATE "BUILD_FOR_MODERN_OS")
    target_link_libraries(isle PRIVATE SDL2 SDL2main)
    target_link_directories(isle PRIVATE ${SDL2_LIBDIR})
    target_include_directories(isle PRIVATE ${SDL2_INCLUDE_DIRS})
  else()
    # Use internal DirectX 5 if required
    target_link_libraries(isle PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:DirectX5::DirectX5>)

    # Link DSOUND, WINMM, and LEGO1
    target_link_libraries(isle PRIVATE dsound winmm)
  endif()

  if (ISLE_USE_SMARTHEAP)
    target_link_libraries(isle PRIVATE SmartHeap::SmartHeap)
  endif()

  # Link DSOUND, WINMM, and LEGO1
  target_link_libraries(isle PRIVATE inifile-cpp::inifile-cpp)
  if(ISLE_BUILD_LEGO1)
    target_link_libraries(isle PRIVATE lego1)
  else()
    target_link_libraries(isle PRIVATE beta10)
  endif()

  # Make sure filenames are ALL CAPS
  set_property(TARGET isle PROPERTY OUTPUT_NAME ISLE)
  set_property(TARGET isle PROPERTY SUFFIX ".EXE")

  install(TARGETS isle DESTINATION .)
endif()

if (ISLE_BUILD_CONFIG)
  file(GLOB_RECURSE configSrc "CONFIG/*.cpp")
  file(GLOB_RECURSE configInc "CONFIG/*.h")
  file(GLOB_RECURSE configRes "CONFIG/*.rc")
  source_group(TREE "${PROJECT_SOURCE_DIR}/CONFIG" PREFIX "Source Files" FILES ${configSrc})
  source_group(TREE "${PROJECT_SOURCE_DIR}/CONFIG" PREFIX "Header Files" FILES ${configInc})
  source_group(TREE "${PROJECT_SOURCE_DIR}/CONFIG/res" PREFIX "Resource Files" FILES ${configRes})
  add_executable(config WIN32
    LEGO1/mxdirectx/mxdirectxinfo.cpp
    LEGO1/mxdirectx/legodxinfo.cpp
    ${configSrc} ${configInc} ${configRes})
  reccmp_add_target(config ID CONFIG)
  target_compile_definitions(config PRIVATE _AFXDLL MXDIRECTX_FOR_CONFIG)
  target_include_directories(config PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/util" "${CMAKE_CURRENT_SOURCE_DIR}/LEGO1")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    target_link_libraries(config PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:DirectX5::DirectX5>)
  endif()
  target_compile_definitions(config PRIVATE DIRECT3D_VERSION=0x500)
  target_link_libraries(config PRIVATE $<$<BOOL:${ISLE_USE_DX5}>:ddraw> dxguid inifile-cpp::inifile-cpp)
  set_property(TARGET config PROPERTY OUTPUT_NAME "CONFIG")
  set_property(TARGET config PROPERTY SUFFIX ".EXE")
  set_property(TARGET config PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)

  if (BUILD_FOR_MODERN_OS)
    target_compile_definitions(config PRIVATE "BUILD_FOR_MODERN_OS")
  endif()

  install(TARGETS config DESTINATION .)
endif()

if (MSVC)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15")
    set_property(TARGET ${lego1_targets} ${beta10_targets} APPEND PROPERTY COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
    if (TARGET isle)
      target_compile_definitions(isle PRIVATE "_CRT_SECURE_NO_WARNINGS")
    endif()
    if (TARGET config)
      target_compile_definitions(config PRIVATE "_CRT_SECURE_NO_WARNINGS")
    endif()
  endif()
  # Visual Studio 2017 version 15.7 needs "/Zc:__cplusplus" for __cplusplus
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.14.26428")
    foreach(tgt IN LISTS lego1_targets beta10_targets)
      target_compile_options(${tgt} PRIVATE "-Zc:__cplusplus")
    endforeach()
    if (TARGET isle)
      target_compile_options(isle PRIVATE "-Zc:__cplusplus")
    endif()
    if (TARGET config)
      target_compile_options(config PRIVATE "-Zc:__cplusplus")
    endif()
  endif()
endif()

if (ISLE_DECOMP_ASSERT)
    message(STATUS "Decomp asserts enabled")
    foreach(tgt IN LISTS lego1_targets beta10_targets)
      target_compile_definitions(${tgt} PRIVATE "ENABLE_DECOMP_ASSERTS")
    endforeach()

    if (TARGET isle)
      target_compile_definitions(isle PRIVATE "ENABLE_DECOMP_ASSERTS")
    endif()
    if (TARGET config)
      target_compile_definitions(config PRIVATE "ENABLE_DECOMP_ASSERTS")
    endif()
endif()

if (MSVC_FOR_DECOMP)
  # These flags have been taken from the defaults for a Visual C++ 4.20 project (the compiler the
  # game was originally built with) and tweaked slightly to produce more debugging info for reccmp.
  # They ensure a recompilation that can be byte/instruction accurate to the original binaries.
  if (ISLE_BUILD_APP)
    target_link_options(isle PRIVATE "/OPT:REF")
    set_property(TARGET isle ${lego1_targets} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()

  if(TARGET lego1)
    target_link_options(lego1 PRIVATE "/OPT:REF")
    # Equivalent to target_compile_options(... PRIVATE "/MT$<$<CONFIG:Debug>:d>")
    set_property(TARGET lego1 ${lego1_targets} ${beta10_targets} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()

  set(CMAKE_CXX_FLAGS "/W3 /GX /D \"WIN32\" /D \"_WINDOWS\"")
  set(CMAKE_CXX_FLAGS_DEBUG "/Gm /Zi /Od /D \"_DEBUG\"")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /D \"NDEBUG\"")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /O2 /D \"NDEBUG\"")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os /D \"NDEBUG\"")

  set(CMAKE_EXE_LINKER_FLAGS "/machine:I386")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/incremental:yes /debug")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/incremental:no")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no /debug")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "/incremental:no")

  set(CMAKE_STATIC_LINKER_FLAGS "/machine:I386")

  set(CMAKE_SHARED_LINKER_FLAGS "/machine:I386")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/incremental:yes /debug")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/incremental:no")
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no /debug")
  set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "/incremental:no")

  # Older MSVC versions don't support building in parallel.
  # Force non-parallel builds of isle and lego1 by putting them in a pool with 1 available job.
  if (CMAKE_CXX_COMPILER_ID VERSION_LESS 12)
    foreach(tgt IN LISTS lego1_targets beta10_targets)
      set_property(GLOBAL APPEND PROPERTY JOB_POOLS "msvc_${tgt}=1")
      set_property(TARGET ${tgt} PROPERTY JOB_POOL_COMPILE "msvc_${tgt}")
      set_property(TARGET ${tgt} PROPERTY JOB_POOL_LINK "msvc_${tgt}")
    endforeach()
    if (TARGET isle)
      set_property(GLOBAL APPEND PROPERTY JOB_POOLS "msvc_isle=1")
      set_property(TARGET isle PROPERTY JOB_POOL_COMPILE "msvc_isle")
      set_property(TARGET isle PROPERTY JOB_POOL_LINK "msvc_isle")
    endif()
    if (TARGET config)
      set_property(GLOBAL APPEND PROPERTY JOB_POOLS "msvc_config=1")
      set_property(TARGET config PROPERTY JOB_POOL_COMPILE "msvc_config")
      set_property(TARGET config PROPERTY JOB_POOL_LINK "msvc_config")
    endif()
  endif()
else()
  if (ISLE_BUILD_APP)
    target_link_options(isle PRIVATE "/SAFESEH:NO")
  endif()

  if (NOT BUILD_FOR_MODERN_OS)
    target_link_options(lego1 PRIVATE "/SAFESEH:NO")
  endif()
endif()

find_program(CLANGFORMAT_BIN NAMES clang-format)
if(EXISTS "${CLANGFORMAT_BIN}")
  execute_process(COMMAND "${CLANGFORMAT_BIN}" --version
    OUTPUT_VARIABLE "CLANGFORMAT_VERSION_OUTPUT"
    RESULT_VARIABLE "CLANGFORMAT_RESULT"
  )
  if(CLANGFORMAT_RESULT EQUAL 0 AND CLANGFORMAT_VERSION_OUTPUT MATCHES "version ([0-9\\.]+)")
    set(CLANGFORMAT_VERSION "${CMAKE_MATCH_1}")
    set(CLANGFORMAT_VERSION_REQUIRED "17.0")
    message(DEBUG "Found clang-format version ${CLANGFORMAT_VERSION} (needs ${CLANGFORMAT_VERSION_REQUIRED}")
    if(CLANGFORMAT_VERSION VERSION_GREATER_EQUAL "${CLANGFORMAT_VERSION_REQUIRED}")
      file(GLOB_RECURSE isle_sources
        "${PROJECT_SOURCE_DIR}/ISLE/*.cpp"
        "${PROJECT_SOURCE_DIR}/ISLE/*.h"
        "${PROJECT_SOURCE_DIR}/LEGO1/*.cpp"
        "${PROJECT_SOURCE_DIR}/LEGO1/*.h"
      )
      string(REPLACE ";" "\n" isle_sources_lines "${isle_sources}")
      file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/isle_sources.txt" "${isle_sources_lines}\n")
      add_custom_target(clang-format ${CLANGFORMAT_BIN} -i "--files=${CMAKE_CURRENT_BINARY_DIR}/isle_sources.txt")
    endif()
  endif()
endif()

reccmp_configure()
