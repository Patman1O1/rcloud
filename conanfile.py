from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.scm import Git
import os

class RcloudFile(ConanFile):
    name = "rcloud"
    version = "0.1.0"
    author = "Patman1O1"
    description = ""

    settings = ("os", "arch", "compiler", "build_type")

    exports_sources = ("CMakeLists.txt", "cmake/*", "src/*")

    options = {
        "build_shared_libs": [True, False],
        "build_tests": [True, False]
    }

    default_options = {
        "build_shared_libs": False,
        "build_tests": False
    }

    def build_requirements(self) -> None:
        self.tool_requires("cmake/[>=3.28.0]")
        self.tool_requires("ccache/[>=4.8.3]")
        self.tool_requires("cppcheck/[>=2.12.1]")
        if bool(self.options.build_tests):
            self.test_requires("gtest/1.14.0")

    def layout(self) -> None:
        cmake_layout(self)

    def generate(self) -> None:
        toolchain = CMakeToolchain(self)
        toolchain.variables["BUILD_SHARED_LIBS"] = bool(self.options.build_shared_libs)
        toolchain.variables["BUILD_TESTS"] = bool(self.options.build_tests)
        toolchain.generate()
        CMakeDeps(self).generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self) -> None:
        CMake(self).install()

    def source(self):
        if not os.path.exists(os.path.join(self.source_folder, "CMakeLists.txt")):
            git = Git(self)
            git.clone("https://github.com/Patman1O1/rcloud.git", target=".")
            git.checkout("v0.1.0")
