# Set the default value of BUILD_SHARED_LIBS to ON
option(BUILD_SHARED_LIBS "Build the library as a shared library" ON)

# Define the primary target as a shared library
add_library(${PROJECT_PRIMARY_TARGET})
add_library(${PROJECT_NAMESPACE}::${PROJECT_PRIMARY_TARGET} ALIAS ${PROJECT_PRIMARY_TARGET})

# Create objects
add_library(${PROJECT_PRIMARY_TARGET}_objects OBJECT "${CMAKE_SOURCE_DIR}/src/${PROJECT_OUTPUT_NAME}/${PROJECT_PRIMARY_TARGET}.c")
add_library(${PROJECT_NAMESPACE}::${PROJECT_PRIMARY_TARGET}_objects ALIAS ${PROJECT_PRIMARY_TARGET}_objects)

# Include directories
target_include_directories(${PROJECT_PRIMARY_TARGET}_objects
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/${PROJECT_OUTPUT_NAME}"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(${PROJECT_PRIMARY_TARGET} PRIVATE $<TARGET_OBJECTS:${PROJECT_PRIMARY_TARGET}_objects>)

# Link objects with the primary target
target_link_libraries(${PROJECT_PRIMARY_TARGET} PRIVATE ${PROJECT_PRIMARY_TARGET}_objects)

# Initialize export files
include(GenerateExportHeader)
set(EXPORT_HEADER_FILE)
set(EXPORT_TARGET_FILE)

if(BUILD_SHARED_LIBS)
    # Set the primary target's properties
    set_target_properties(${PROJECT_PRIMARY_TARGET} PROPERTIES
            OUTPUT_NAME ${PROJECT_OUTPUT_NAME}
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            C_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
    )

    # Define compile definitions
    target_compile_definitions(${PROJECT_PRIMARY_TARGET}
            PUBLIC
                $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${PROJECT_NAME_SCREAMING_CASE}_STATIC_DEFINE>
    )

    # Set export files
    set(EXPORT_HEADER_FILE "export_shared.h")
    set(EXPORT_TARGET_FILE "${PROJECT_PACKAGE_NAME}SharedTargets.cmake")
else()
    # Set the primary target's properties
    set_target_properties(${PROJECT_PRIMARY_TARGET} PROPERTIES
            OUTPUT_NAME ${PROJECT_OUTPUT_NAME}
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
            POSITION_INDEPENDENT_CODE ON
    )

    # Define compile definitions
    target_compile_definitions(${PROJECT_PRIMARY_TARGET}
            PUBLIC
                ${PROJECT_NAME_SCREAMING_CASE}_STATIC_DEFINE
    )

    # Set export files
    set(EXPORT_HEADER_FILE "export_static.h")
    set(EXPORT_TARGET_FILE "${PROJECT_EXPORT_NAME}StaticTargets.cmake")
endif()

# Generate export header file
generate_export_header(${PROJECT_PRIMARY_TARGET} EXPORT_FILE_NAME "include/${PROJECT_OUTPUT_NAME}/${EXPORT_HEADER_FILE}")

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

    # Create export files
    include(GenerateExportHeader)
    set(EXPORT_HEADER_FILE "export_static.h")
    set(EXPORT_TARGET_FILE "${PROJECT_EXPORT_NAME}StaticTargets.cmake")
    if(BUILD_SHARED_LIBS)
        set(EXPORT_HEADER_FILE "export_shared.h")
        set(EXPORT_TARGETS_FILE "${PROJECT_PACKAGE_NAME}SharedTargets.cmake")
    endif()

    # Install the library target
    install(TARGETS ${PROJECT_PRIMARY_TARGET} ${PROJECT_PRIMARY_TARGET}_objects EXPORT ${PROJECT_PRIMARY_TARGET}_export
            RUNTIME COMPONENT ${PROJECT_OUTPUT_NAME}
            LIBRARY COMPONENT ${PROJECT_OUTPUT_NAME} NAMELINK_COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            ARCHIVE COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    # Install the library headers
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
            TYPE INCLUDE
            COMPONENT ${PROJECT_OUTPUT_NAME}-dev)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_OUTPUT_NAME}/${EXPORT_HEADER_FILE}"
            COMPONENT ${PROJECT_OUTPUT_NAME}-dev
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_OUTPUT_NAME}")

    # Install export files
    install(EXPORT ${PROJECT_PRIMARY_TARGET}_export
            COMPONENT mylib-dev
            FILE "${EXPORT_TARGETS_FILE}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_OUTPUT_NAME}"
            NAMESPACE ${PROJECT_NAMESPACE}::)

endif()
