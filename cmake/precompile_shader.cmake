function(precompile_shader SOURCE_SHADER_PATH DESTINATION_SHADER_PATH)
    if(NOT EXISTS "${SOURCE_SHADER_PATH}")
        message(FATAL_ERROR "Source shader does not exist: ${SOURCE_SHADER_PATH}")
    endif()

    get_filename_component(SOURCE_DIR "${SOURCE_SHADER_PATH}" DIRECTORY)
    get_filename_component(DEST_DIR "${DESTINATION_SHADER_PATH}" DIRECTORY)
    get_filename_component(SHADER_FILENAME "${DESTINATION_SHADER_PATH}" NAME_WE)

    file(MAKE_DIRECTORY "${DEST_DIR}")

    # Precompile the shader
    add_custom_command(
            OUTPUT "${DESTINATION_SHADER_PATH}"
            COMMAND glslangValidator -V "${SOURCE_SHADER_PATH}" -o "${DESTINATION_SHADER_PATH}"
            DEPENDS "${SOURCE_SHADER_PATH}"
            COMMENT "Precompiling shader: ${SOURCE_SHADER_PATH} to ${DESTINATION_SHADER_PATH}"
    )

    add_custom_target(precompile_${SHADER_FILENAME}
            DEPENDS "${DESTINATION_SHADER_PATH}"
    )
endfunction()