#-----------------------------------------------------------------------------------------------------------------------
# Target Name: rcloud_test
# Target Type: Executable
# Target Dependencies: rcloud_objects
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(${PROJECT_PRIMARY_TARGET}_test)

# Set properties
set_target_properties(rcloud_test PROPERTIES
        OUTPUT_NAME rcloud-test
)

# Include directories
target_include_directories(rcloud_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(rcloud_test
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/rcloud_test.cpp"
)

target_link_libraries(rcloud_test
        PRIVATE
            $<$<TARGET_EXISTS:rcloud_objects>:rcloud_objects>
            GTest::gtest_main
            GTest::gmock_main
)
gtest_discover_tests(rcloud_test)
