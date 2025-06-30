find_package(glm CONFIG REQUIRED)
find_package(OpenXR CONFIG REQUIRED)


add_library(XRModule STATIC
    xreality.h
)

set_target_properties(XRModule
        PROPERTIES
        OUTPUT_NAME "gyroxr"
)

target_link_libraries(XRModule PUBLIC
        UtilitiesModule
        OpenXR::headers
        OpenXR::openxr_loader
)

target_include_directories(XRModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)