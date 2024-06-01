import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.build import check_min_cppstd


class bbRecipe(ConanFile):
    name = "bb"
    version = "1.0"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Luke X."
    url = "<Package recipe repository url here, for issues about the package>"
    description = "A boring example project for booking fake movie tickets online"
    topics = ("C++", "API", "demo")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "include/*", "src/*", "tests/*"

    def validate(self):
        check_min_cppstd(self, "17")

    def requirements(self):
        self.requires("cpp-httplib/0.15.3")
        self.test_requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if not self.conf.get("tools.build:skip_test", default=False):
            test_folder = os.path.join("tests")
            if self.settings.os == "Windows":
                test_folder = os.path.join("tests", str(self.settings.build_type))
            self.run(os.path.join(test_folder, "test_bb"))

    def package(self):
        cmake = CMake(self)
        cmake.install()

    

    
