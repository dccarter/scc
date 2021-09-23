cmake_minimum_required(VERSION 3.14)
project(Scc VERSION 0.1.0 LANGUAGES C CXX)

# Configure for C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Configure build options
option(SCC_BUILD_DEMO "Enbable/Disable building the demo library and app" ON)

find_package(Threads REQUIRED)

add_library(Scc STATIC
        src/generator.cpp
        src/generator2.cpp
        src/includes.cpp
        src/program.cpp
        src/formatter.cpp
        src/exception.cpp)
add_library(Suil::Scc ALIAS Scc)

set_target_properties(Scc
        PROPERTIES OUTPUT_NAME scc)
target_include_directories(Scc PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(Scc
        PUBLIC Threads::Threads)

add_executable(scc-bin
        src/main.cpp
        src/meta.cpp
        src/parser.cpp
        src/program_generator.cpp)
set_target_properties(scc-bin
        PROPERTIES OUTPUT_NAME scc)

target_link_libraries(scc-bin Suil::Scc dl)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    if (SCC_BUILD_DEMO)
        include(FetchContent)
        FetchContent_Declare(
                iod
                GIT_REPOSITORY https://gitlab.com/sw-devel/thirdparty/iod.git)
        FetchContent_MakeAvailable(iod)

        add_library(demo-lib SHARED
            demo/demo.cpp)
        set_target_properties(demo-lib
                PROPERTIES OUTPUT_NAME demo)
        target_link_libraries(demo-lib Suil::Scc)

        # Add demo binary application
        add_executable(demo
            ${CMAKE_CURRENT_SOURCE_DIR}/demo/code.scc.cpp
            demo/main.cpp)
        target_link_libraries(demo Iod::Iod)

        # Include SuilScc
        include(cmake/SuilScc.cmake)

        # Add target to compile SCC sources
        set(DEMO_SCC_SOURCES
                demo/code.scc
                demo/symbols.scc)
        SuilScc(demo
            BINARY   ${CMAKE_CURRENT_BINARY_DIR}/scc
            DEPENDS  scc-bin demo-lib
            SOURCES  ${DEMO_SCC_SOURCES}
            LIB_PATH ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endif()

include(CMakePackageConfigHelpers)

set(GENERATED_DIR  ${CMAKE_BINARY_DIR}/generated)
set(VERSION_CONFIG ${GENERATED_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
set(PROJECT_CONFIG ${GENERATED_DIR}/${PROJECT_NAME}Config.cmake)
set(SCC_TARGETS_EXPORT_NAME  ${PROJECT_NAME}Targets)
set(NAMESPACE Suil::)

# Configure '<PROJECT-NAME>Config.cmake.in'
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        ${PROJECT_CONFIG}
        INSTALL_DESTINATION lib/cmake/Suil${PROJECT_NAME})

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
write_basic_package_version_file(
        "${VERSION_CONFIG}" COMPATIBILITY SameMajorVersion)

install(TARGETS Scc
        EXPORT  ${SCC_TARGETS_EXPORT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(TARGETS scc-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

set(LIBRARY_HEADERS
        include/scc/exception.hpp
        include/scc/formatter.hpp
        include/scc/generator.hpp
        include/scc/includes.hpp
        include/scc/program.hpp
        include/scc/visitor.hpp)

# Install library header files
install(FILES ${LIBRARY_HEADERS}
        DESTINATION include/scc)
# Might as well install clipp
install(FILES include/scc/clipp.hpp
        DESTINATION include)

install(FILES cmake/SuilScc.cmake
        DESTINATION lib/cmake/)

# Install config
#   * <prefix>/lib/cmake/SuilDbConfig.cmake
#   * <prefix>/lib/cmake/SuilDbConfigVersion.cmake
install(FILES ${PROJECT_CONFIG} ${VERSION_CONFIG}
        DESTINATION lib/cmake/${PROJECT_NAME})

# Install cmake targets
#   * <prefix>/lib/cmake/SuilDbTargets.cmake
install(EXPORT      ${SCC_TARGETS_EXPORT_NAME}
        NAMESPACE   ${NAMESPACE}
        DESTINATION lib/cmake/${PROJECT_NAME})

set(CPACK_PACKAGE_NAME   ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Suilteam")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Carter Mbotho")
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CMAKE_PROJECT_VERSION}-${CMAKE_BUILD_TYPE})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_GENERATOR "TGZ")
include(CPack)