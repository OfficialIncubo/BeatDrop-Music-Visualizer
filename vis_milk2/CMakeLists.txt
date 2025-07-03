cmake_minimum_required(VERSION 3.25)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)
project(vis_milk2)

option(ENABLE_PROJECTM_NSEEL "Enable the use of a system-installed/external projectM-nseel library" ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_STATIC_LIBRARY_PREFIX lib)

if(ENABLE_PROJECTM_NSEEL)
    list(APPEND VCPKG_MANIFEST_FEATURES external-nseellib)
endif()

#Thank you, Kai Blaschke (CodAv)!
if(ENABLE_PROJECTM_NSEEL)
    find_package(projectM-Eval REQUIRED COMPONENTS Milkdrop)

    set (SOURCES
            dxcontext.cpp
            fft.cpp
            milkdropfs.cpp
            plugin.cpp
            pluginshell.cpp
            state.cpp
            support.cpp
            texmgr.cpp
            utility.cpp
            plugin.h
            state.h
            texmgr.h
            )

if (BUILD_SHARED_LIBS)
add_library(vis_milk2 SHARED ${SOURCES})
else()
add_library(vis_milk2 STATIC ${SOURCES})
endif()

add_executable(vis_milk2
    ${SOURCES}
)

target_include_directories(vis_milk2
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ns-eel2
    )

target_link_libraries(vis_milk2 PUBLIC projectM::ns-eel2)

#    set_target_properties(vis_milk2 PROPERTIES
#                      EXPORT_NAME ns-eel2
#                      )

#    install(TARGETS vis_milk2
#            DESTINATION ${CMAKE_INSTALL_LIBDIR}
#            EXPORT projectM-EvalMilkdrop
#            )

#    install(EXPORT projectM-EvalMilkdrop
#            FILE projectM-EvalMilkdropTargets.cmake
#            NAMESPACE projectM::
#            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/projectM-EvalMilkdrop"
#            COMPONENT Devel
#            )
endif()

#if(NOT TARGET projectM::ns-eel2)
#    add_subdirectory(projectm-eval)
#endif()