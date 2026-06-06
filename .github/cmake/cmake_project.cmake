#-----------------------------------------------------------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/functions.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Parameters (Required)
#-----------------------------------------------------------------------------------------------------------------------
set(PROJECT_NAME "" CACHE STRING "The name of the project")
set(PROJECT_AUTHOR "" CACHE STRING "The name of the author")
set(PROJECT_TYPE "" CACHE STRING "The type of the project")

#-----------------------------------------------------------------------------------------------------------------------
# Parameters (Optional)
#-----------------------------------------------------------------------------------------------------------------------
set(PROJECT_VERSION 0.1.0 CACHE STRING "The version of the project")
set(PROJECT_DESCRIPTION "" CACHE STRING "The description of the project")

#-----------------------------------------------------------------------------------------------------------------------
# Parameter Validation
#-----------------------------------------------------------------------------------------------------------------------
# Validate the project name
if(NOT PROJECT_NAME)
    message(FATAL_ERROR "PROJECT_NAME was not specified")
    return()
endif()

# Validate the project author
if (NOT PROJECT_AUTHOR)
    message(FATAL_ERROR "PROJECT_AUTHOR was not specified")
endif()

# Format the project type in snake_case
to_snake_case("${PROJECT_TYPE}" PROJECT_TYPE)

# Validate the project type and set BUILD_SHARED_LIBS if the project type is a shared library
if(PROJECT_TYPE MATCHES "^(executable|static_library|interface_library)$")
    set(BUILD_SHARED_LIBS OFF)
    set(CONAN_BUILD_SHARED_LIBS False)
elseif(PROJECT_TYPE MATCHES "shared_library")
    set(BUILD_SHARED_LIBS ON)
    set(CONAN_BUILD_SHARED_LIBS True)
else()
    message(FATAL_ERROR "PROJECT_TYPE was not specified as Executable, Static Library, Shared Library, or Interface Library")
    return()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Variables
#-----------------------------------------------------------------------------------------------------------------------
# CMake Variables
set(CMAKE_MINIMUM_REQUIRED_VERSION 3.28.0)

# C Variables
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# C++ Variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project Variables
to_pascal_case("${PROJECT_NAME}" PROJECT_NAME_PASCAL_CASE)
to_snake_case("${PROJECT_NAME}" PROJECT_NAME_SNAKE_CASE)
to_kebab_case("${PROJECT_NAME}" PROJECT_NAME_KEBAB_CASE)
string(TOUPPER "${PROJECT_NAME_SNAKE_CASE}" PROJECT_NAME_SCREAMING_CASE)
set(PROJECT_NAMESPACE ${PROJECT_NAME_SNAKE_CASE})
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
set(PROJECT_PRIMARY_TARGET ${PROJECT_NAME_SNAKE_CASE} CACHE STRING "The primary target this project will build")
set(PROJECT_OUTPUT_NAME ${PROJECT_NAME_KEBAB_CASE} CACHE STRING "The base name for output files created for the primary target")
set(PROJECT_PACKAGE_NAME ${PROJECT_NAME_PASCAL_CASE} CACHE STRING "The name that will be displayed on package related files")

#-----------------------------------------------------------------------------------------------------------------------
# CMake Directory Configuration
#-----------------------------------------------------------------------------------------------------------------------
# Rename cmake/TemplateConfig.cmake.in to cmake/${PROJECT_PACKAGE_NAME}Config.cmake.in
file(RENAME "${PROJECT_ROOT_DIR}/cmake/TemplateConfig.cmake.in"
            "${PROJECT_ROOT_DIR}/cmake/${PROJECT_PACKAGE_NAME}Config.cmake.in")

#-----------------------------------------------------------------------------------------------------------------------
# Project Root Directory Configuration
#-----------------------------------------------------------------------------------------------------------------------
# Configure the project root CMakeLists.txt file
configure_file("${PROJECT_ROOT_DIR}/CMakeLists.txt.in"
               "${PROJECT_ROOT_DIR}/CMakeLists.txt"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/CMakeLists.txt.in")

# Configure the conanfile.py file
configure_file("${PROJECT_ROOT_DIR}/conanfile.py.in"
               "${PROJECT_ROOT_DIR}/conanfile.py"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/conanfile.py.in")

#-----------------------------------------------------------------------------------------------------------------------
# Project Include Directory Configuration
#-----------------------------------------------------------------------------------------------------------------------
# Rename the template subdirectory to the project name
file(RENAME "${PROJECT_ROOT_DIR}/include/template" "${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}")

# Configure the project header file (i.e. ${PROJECT_PRIMARY_TARGET}.h)
configure_file("${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/template.h.in"
               "${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/${PROJECT_PRIMARY_TARGET}.h"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/template.h.in")

# Configure the export header file (i.e. export.h)
configure_file("${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/export.h.in"
               "${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/export.h"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/include/${PROJECT_OUTPUT_NAME}/export.h.in")

#-----------------------------------------------------------------------------------------------------------------------
# Project Source Directory Configuration
#-----------------------------------------------------------------------------------------------------------------------
# Rename ${PROJECT_ROOT_DIR}/src/template to ${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}
file(RENAME "${PROJECT_ROOT_DIR}/src/template" "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}")

# Configure the source directory CMakeLists.txt
configure_file("${PROJECT_ROOT_DIR}/src/CMakeLists.txt.in"
               "${PROJECT_ROOT_DIR}/src/CMakeLists.txt"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/src/CMakeLists.txt.in")

# Configure the CMakeLists.txt file in ${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}
file(READ "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_TYPE}_definition.cmake" PROJECT_PRIMARY_TARGET_INIT)
configure_file("${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/CMakeLists.txt.in"
               "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/CMakeLists.txt"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/CMakeLists.txt.in")

if(PROJECT_TYPE MATCHES "^(static_library|shared_library)$")
    # Remove main.c.in (libraries don't have entry points)
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c.in")

    # Configure ${PROJECT_PRIMARY_TARGET}.c
    configure_file("${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/template.c.in"
                   "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/${PROJECT_PRIMARY_TARGET}.c"
                   @ONLY)
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/template.c.in")
elseif(PROJECT_TYPE MATCHES "interface_library")
    # Remove all source files
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/template.c.in")
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c.in")
else()
    # Configure main.c
    configure_file("${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c.in"
                   "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c"
                   @ONLY)
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/main.c.in")

    # Configure ${PROJECT_PRIMARY_TARGET}.c
    configure_file("${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/template.c.in"
                   "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/${PROJECT_PRIMARY_TARGET}.c"
                   @ONLY)
    file(REMOVE "${PROJECT_ROOT_DIR}/src/${PROJECT_OUTPUT_NAME}/template.c.in")
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Project Test Directory Configuration
#----------------------------------------------------------------------------------------------------------------------
# Rename ${PROJECT_ROOT_DIR}/test/template to ${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}
file(RENAME "${PROJECT_ROOT_DIR}/test/template" "${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}")

# Configure the test directory CMakeLists.txt
configure_file("${PROJECT_ROOT_DIR}/test/CMakeLists.txt.in"
               "${PROJECT_ROOT_DIR}/test/CMakeLists.txt"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/test/CMakeLists.txt.in")

# Configure the ${PROJECT_OUTPUT_NAME} CMakeLists.txt file
configure_file("${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/CMakeLists.txt.in"
               "${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/CMakeLists.txt"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/CMakeLists.txt.in")

# Configure the ${PROJECT_PRIMARY_TARGET} .cpp file
configure_file("${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/template_test.cpp.in"
               "${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/${PROJECT_PRIMARY_TARGET}_test.cpp"
               @ONLY)
file(REMOVE "${PROJECT_ROOT_DIR}/test/${PROJECT_OUTPUT_NAME}/template_test.cpp.in")
