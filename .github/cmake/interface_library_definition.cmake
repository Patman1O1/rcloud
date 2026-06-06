# Define the primary target as an interface library
add_library(${PROJECT_PRIMARY_TARGET} INTERFACE)
add_library(${PROJECT_NAMESPACE}::${PROJECT_PRIMARY_TARGET} ALIAS ${PROJECT_PRIMARY_TARGET})

# Set properties
set_target_properties(${PROJECT_PRIMARY_TARGET} PROPERTIES
        OUTPUT_NAME ${PROJECT_OUTPUT_NAME}
)

# Initialize export files
include(GenerateExportHeader)
set(EXPORT_HEADER_FILE)
set(EXPORT_TARGET_FILE)

# Set the primary target's properties
set_target_properties(${PROJECT_PRIMARY_TARGET} PROPERTIES
        OUTPUT_NAME ${PROJECT_OUTPUT_NAME}
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
)

if(NOT CMAKE_SKIP_INSTALL_RULES)
    # Generate the configuration file that includes the project exports
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
            "${CMAKE_SOURCE_DIR}/cmake/${PROJECT_PACKAGE_NAME}Config.cmake.in"
            "${CMAKE_BINARY_DIR}/${PROJECT_PACKAGE_NAME}Config.cmake"
            INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_OUTPUT_NAME}"
            NO_SET_AND_CHECK_MACRO
            NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Generate the version file for the configuration file
    write_basic_package_version_file(
            "${CMAKE_BINARY_DIR}/${PROJECT_PACKAGE_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    # Install package files
    install(FILES
            "${CMAKE_BINARY_DIR}/${PROJECT_PACKAGE_NAME}Config.cmake"
            "${CMAKE_BINARY_DIR}/${PROJECT_PACKAGE_NAME}ConfigVersion.cmake"
            COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_OUTPUT_NAME}")

    # Install the library target
    install(TARGETS ${PROJECT_PRIMARY_TARGET}
            RUNTIME COMPONENT ${PROJECT_OUTPUT_NAME}
            LIBRARY COMPONENT ${PROJECT_OUTPUT_NAME} NAMELINK_COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            ARCHIVE COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    # Install the library headers
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
            TYPE INCLUDE
            COMPONENT ${PROJECT_OUTPUT_NAME}-dev)
endif()
