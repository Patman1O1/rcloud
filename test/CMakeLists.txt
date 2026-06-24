#-----------------------------------------------------------------------------------------------------------------------
# Information
#-----------------------------------------------------------------------------------------------------------------------
enable_language(CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-----------------------------------------------------------------------------------------------------------------------
# Google Test
#-----------------------------------------------------------------------------------------------------------------------
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: rcloud_test_utils
# Target Type: Interface Library
#-----------------------------------------------------------------------------------------------------------------------
# Define the target
add_library(rcloud_test_utils INTERFACE)

# Set properties
set_target_properties(rcloud_test_utils PROPERTIES
        OUTPUT_NAME rcloud-test-utils
)

# Include directories
target_include_directories(rcloud_test_utils
        INTERFACE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Publicly link with Google Test and Google Mock
target_link_libraries(rcloud_test_utils
        INTERFACE
            GTest::gtest_main
            GTest::gmock_main
)

#-----------------------------------------------------------------------------------------------------------------------
# Target Name: fs_test
# Target Type: Executable
#-----------------------------------------------------------------------------------------------------------------------
# Define the test target
add_executable(fs_test)

# Set properties
set_target_properties(fs_test PROPERTIES
        OUTPUT_NAME fs-test
)

# Include directories
target_include_directories(fs_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/include/rcloud"
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Set sources
target_sources(fs_test
        PRIVATE
            "${CMAKE_SOURCE_DIR}/src/rcloud/fs.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/fs_test.cpp"
)

target_link_libraries(fs_test
        PRIVATE
            rcloud_test_utils
)
gtest_discover_tests(fs_test)

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
            "${CMAKE_SOURCE_DIR}/src/rcloud/fs.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/backing_file.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/backing_file_test.cpp"
)

target_link_libraries(backing_file_test
        PRIVATE
            rcloud_test_utils
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
            "${CMAKE_SOURCE_DIR}/src/rcloud/fs.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/backing_file.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/loop.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/loop_test.cpp"
)

target_link_libraries(loop_test
        PRIVATE
            rcloud_test_utils
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
            "${CMAKE_SOURCE_DIR}/src/rcloud/fs.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/backing_file.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/loop.c"
            "${CMAKE_SOURCE_DIR}/src/rcloud/drive.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/drive_test.cpp"
)

target_link_libraries(drive_test
        PRIVATE
            rcloud_test_utils
)
gtest_discover_tests(drive_test)
