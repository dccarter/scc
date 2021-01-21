if (NOT SUIL_SCC_DEFAULT_BINARY)
    set(SUIL_SCC_DEFAULT_BINARY scc)
endif()

if (NOT TARGET Suil::Scc)
    find_package(Scc REQUIRED)
endif()

#
#! SuilScc : This function takes a collection of suil code scripts for a specific
#  target and transpiles them into C/C++ code
#
# \arg:name the name of the target associated with the scripts. This will be used to
#  to generate the transpiling target as ${name}-scc
# \group:DEPENDS a list of dependencies to add to the generating target
# \group:SOURCES a list of scc source file to be transpiled (default: CMAKE_CURRENT_SOURCE_DIR/${name}.scc)
# \param:OUTDIR the output directory for the scripts(default: the directory where each script is located)
# \param:BINARY path to scc binary (default: system scc)
#
function(SuilScc name)
    set(options PROJECT PRIVATE)
    set(kvargs  BINARY OUTDIR LIB_PATH)
    set(kvvargs DEPENDS SOURCES)

    cmake_parse_arguments(SUIL_SCC "${options}" "${kvargs}" "${kvvargs}" ${ARGN})

    # Configure suil code compiler binary
    set(scc ${SUIL_SCC_DEFAULT_BINARY})
    if (SUIL_SCC_BINARY)
        set(scc ${SUIL_SCC_BINARY})
    endif()

    # Locate source's
    set(${name}_SOURCES ${SUIL_SCC_SOURCES})
    if (NOT SUIL_SCC_SOURCES)
        set(${name}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${name}.scc)
    endif()

    set(_name ${name})
    if (SUIL_SCC_PRIVATE)
        set(_name ${name}-priv)
    endif()

    # Generate source files
    set(${name}_OUTPUTS)
    if (SUIL_SCC_OUTDIR)
        set(${name}_SCC_SOURCES)
        foreach(__${name}_SOURCE ${${name}_SOURCES})
            get_filename_component(__temp ${__${name}_SOURCE} NAME)
            list(APPEND ${name}_OUTPUTS
                    ${SUIL_SCC_OUTDIR}/${__temp}.hpp
                    ${SUIL_SCC_OUTDIR}/${__temp}.cpp)
            get_filename_component(__temp ${__${name}_SOURCE} ABSOLUTE)
            list(APPEND ${name}_SCC_SOURCES ${__temp})
        endforeach()

        if (SUIL_SCC_LIB_PATH)
            set(__prefix ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${SUIL_SCC_LIB_PATH}")
        endif()

        add_custom_command(OUTPUT ${${name}_OUTPUTS}
                COMMAND           ${__prefix} ${scc} "build" "--outdir" "${SUIL_SCC_OUTDIR}" ${${name}_SCC_SOURCES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS           ${${name}_SCC_SOURCES})
    else()
        set(${name}_SCC_SOURCES)
        foreach(__${name}_SOURCE ${${name}_SOURCES})
            get_filename_component(__source ${__${name}_SOURCE} ABSOLUTE)
            list(APPEND ${name}_OUTPUTS
                    ${__source}.hpp
                    ${__source}.cpp)
            list(APPEND ${name}_SCC_SOURCES ${__source})
        endforeach()

        if (SUIL_SCC_LIB_PATH)
            set(__prefix ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${SUIL_SCC_LIB_PATH}")
        endif()

        add_custom_command(OUTPUT ${${name}_OUTPUTS}
                COMMAND     ${__prefix} ${scc} "build" ${${name}_SCC_SOURCES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS           ${${name}_SCC_SOURCES})
    endif()

    # Add scc target to generate targets
    add_custom_target(${_name}-scc
            DEPENDS ${${name}_OUTPUTS}
            COMMENT "Generating scc sources used by ${name}")

    if (SUIL_SCC_DEPENDS)
        add_dependencies(${_name}-scc ${SUIL_SCC_DEPENDS})
    endif()

    if (NOT SUIL_SCC_PROJECT)
        add_dependencies(${name} ${_name}-scc)
    endif()
endfunction(SuilScc)

#
#! SuilSccGenerator : This function creates a target that will build an SccGenerator
#
# \arg:name the name of the target to add
# \group:DEPENDS a list of dependencies to add to the target
# \group:SOURCES a list of C/C++ source files to be compiled into a generator library
# \param:OUTPUT_NAME the output of the generated library (required)
#
function(SuilSccGenerator name)
    set(options "")
    set(kvargs  OUTPUT_NAME INSTALL_DIR)
    set(kvvargs SOURCES LIBRARIES DEPENDS)

    cmake_parse_arguments(SUIL_SCC_GEN "${options}" "${kvargs}" "${kvvargs}" ${ARGN})

    if (NOT SUIL_SCC_GEN_SOURCES)
        # At least 1 source file is required
        message(FATAL_ERROR "SuilSccGenerator - missing required SOURCES")
    endif()
    # Add shared library target
    add_library(${name} SHARED
            ${SUIL_SCC_GEN_SOURCES})
    # Target should depend on SCC lib
    target_link_libraries(${name} Suil::Scc)

    if (SUIL_SCC_GEN_LIBRARIES)
        # Additional libraries if any
        target_link_libraries(${name} ${SUIL_SCC_GEN_LIBRARIES})
    endif()

    if (SUIL_SCC_GEN_DEPENDS)
        # Add targets that must be executed before this target
        add_dependencies(${name} ${SUIL_SCC_GEN_DEPENDS})
    endif()

    if (SUIL_SCC_GEN_OUTPUT_NAME)
        # Set the name of the generated binary
        set_target_properties(${name}
                PROPERTIES OUTPUT_NAME ${SUIL_SCC_GEN_OUTPUT_NAME})
    endif()

    if (SUIL_SCC_GEN_INSTALL_DIR)
        install(TARGETS ${name}
                LIBRARY DESTINATION ${SUIL_SCC_GEN_INSTALL_DIR})
    else()
        install(TARGETS ${name}
                LIBRARY DESTINATION lib)
    endif()
endfunction()