# Define the primary target as an executable
add_executable(${PROJECT_PRIMARY_TARGET})

# Set properties
set_target_properties(${PROJECT_PRIMARY_TARGET} PROPERTIES
        OUTPUT_NAME ${PROJECT_OUTPUT_NAME}
)

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
target_sources(${PROJECT_PRIMARY_TARGET}
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c"
            $<TARGET_OBJECTS:${PROJECT_PRIMARY_TARGET}_objects>
)

# Link objects with the primary target
target_link_libraries(${PROJECT_PRIMARY_TARGET} PRIVATE ${PROJECT_PRIMARY_TARGET}_objects)

# Define installation rules
if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_PRIMARY_TARGET} ${PROJECT_PRIMARY_TARGET}_objects
            RUNTIME COMPONENT ${PROJECT_PRIMARY_TARGET}
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()
