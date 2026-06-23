#-----------------------------------------------------------------------------------------------------------------------
# Target Name: util_test
# Target Type: Executable
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(util_test)

# Set properties
set_target_properties(util_test PROPERTIES
        OUTPUT_NAME util-test
)

# Include directories
target_include_directories(util_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(util_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/util.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/util_test.cpp"
)

target_link_libraries(util_test
        PRIVATE
            GTest::gtest_main
            GTest::gmock_main
)
gtest_discover_tests(util_test)

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: backing_file_test
# Target Type: Executable
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(backing_file_test)

# Set properties
set_target_properties(backing_file_test PROPERTIES
        OUTPUT_NAME backing-file-test
)

# Include directories
target_include_directories(backing_file_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(backing_file_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/util.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/backing_file.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/backing_file_test.cpp"
)

target_link_libraries(backing_file_test
        PRIVATE
            GTest::gtest_main
            GTest::gmock_main
)
gtest_discover_tests(backing_file_test)

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: loop_test
# Target Type: Executable
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(loop_test)

# Set properties
set_target_properties(loop_test PROPERTIES
        OUTPUT_NAME loop-test
)

# Include directories
target_include_directories(loop_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(loop_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/util.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/backing_file.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/loop.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/loop_test.cpp"
)

target_link_libraries(loop_test
        PRIVATE
            GTest::gtest_main
            GTest::gmock_main
)
gtest_discover_tests(loop_test)

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: drive_test
# Target Type: Executable
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(drive_test)

# Set properties
set_target_properties(drive_test PROPERTIES
        OUTPUT_NAME drive-test
)

# Include directories
target_include_directories(drive_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(drive_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/drive.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/drive_test.cpp"
)

target_link_libraries(drive_test
        PRIVATE
            GTest::gtest_main
            GTest::gmock_main
)
gtest_discover_tests(drive_test)
