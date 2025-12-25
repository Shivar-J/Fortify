function(fortify_compile_shaders TARGET)
    set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/shaders")
    set(SHADER_OUT_DIR "${CMAKE_BINARY_DIR}/shaders")

    file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

    file(GLOB_RECURSE SHADERS
        "${SHADER_SRC_DIR}/*.vert"
        "${SHADER_SRC_DIR}/*.frag"
        "${SHADER_SRC_DIR}/*.comp"
        "${SHADER_SRC_DIR}/*.rgen"
        "${SHADER_SRC_DIR}/*.rchit"
        "${SHADER_SRC_DIR}/*.rahit"
        "${SHADER_SRC_DIR}/*.rmiss"
        "${SHADER_SRC_DIR}/*.rint"
    )

    set(SPIRV_FILES "")

    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SPIRV_OUT "${SHADER_OUT_DIR}/${SHADER_NAME}.spv")
        list(APPEND SPIRV_FILES ${SPIRV_OUT})

        add_custom_command(
            OUTPUT ${SPIRV_OUT}
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
                    -V ${SHADER}
                    -o ${SPIRV_OUT}
                    --target-env vulkan1.2
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${SHADER_NAME} -> ${SPIRV_OUT}"
            VERBATIM
        )
    endforeach()

    add_custom_target(${TARGET}_shaders ALL
        DEPENDS ${SPIRV_FILES}
    )

    add_dependencies(${TARGET} ${TARGET}_shaders)
endfunction()
