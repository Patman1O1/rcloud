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
# Target Name: google_test_libs
# Target Type: Interface
#-----------------------------------------------------------------------------------------------------------------------
# Define the target
add_library(gtest_libs INTERFACE)

# Link gtest and gmock
target_link_libraries(gtest_libs
        INTERFACE
            GTest::gtest_main
            GTest::gmock_main
)

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

# Publicly link with Google Test and Google Mock
target_link_libraries(drive_test
        PRIVATE
            libmount::libmount
            GTest::gtest_main
            GTest::gmock_main
)

gtest_discover_tests(drive_test)
