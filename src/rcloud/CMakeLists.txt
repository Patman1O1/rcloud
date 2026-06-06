#-----------------------------------------------------------------------------------------------------------------------
# Target Name: rcloud_objects
# Target Type: Object Library
#-----------------------------------------------------------------------------------------------------------------------
# Define the target
add_library(rcloud_objects OBJECT)

# Include directories
target_include_directories(rcloud_objects
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(rcloud_objects
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/filesystem.c"
)

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: rcloud
# Target Type: Executable
# Target Dependencies: rcloud_objects
#-----------------------------------------------------------------------------------------------------------------------
# Define the target
add_executable(rcloud)

# Include directories
target_include_directories(rcloud
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(rcloud
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/main.c"
            $<TARGET_OBJECTS:rcloud_objects>
)

# Link the rcloud object library with the rcloud executable
target_link_libraries(rcloud PRIVATE rcloud_objects)

# Define installation rules
if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(GNUInstallDirs)

    install(TARGETS rcloud rcloud_objects
            RUNTIME COMPONENT rcloud
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
endif()
