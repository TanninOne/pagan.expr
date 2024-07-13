from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake

class SyExprRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        pass

    def build_requirements(self):
        self.tool_requires("cmake/3.27.9")
        self.test_requires("gtest/1.14.0")
        self.test_requires("benchmark/1.8.3")

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"
        self.folders.generators = "generators"

    def generate(self):
        cmake = CMakeToolchain(self)
        cmake.generate()

    def build(self):
        cmake = CMake(self)

        cmake.configure()
        cmake.build()
        if not self.conf.get("tools.build:skip_test", default=False):
            cmake.test()
