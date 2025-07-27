.PHONY: all build clean test lint format help

# Default target executed when no arguments are given to make.
all: help

######################
# BUILD AND CLEAN
######################

# Build the project using CMake
build:
    mkdir -p build && cd build && cmake .. && make -j$(nproc)

# Clean the build directory
clean:
    rm -rf build

######################
# TESTING
######################

# Define a variable for the test executable path
TEST_EXECUTABLE ?= build/tests

# Run unit tests
test:
    $(TEST_EXECUTABLE)

######################
# LINTING AND FORMATTING
######################

# Lint C++ files using clang-tidy
lint:
    find . -name "*.cpp" -o -name "*.h" | xargs clang-tidy --quiet

# Format C++ files using clang-format
format:
    find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

######################
# HELP
######################

help:
    @echo '===================='
    @echo '-- BUILD --'
    @echo 'build                        - build the project using CMake'
    @echo 'clean                        - clean the build directory'
    @echo '-- TESTS --'
    @echo 'test                         - run unit tests'
    @echo '-- LINTING AND FORMATTING --'
    @echo 'lint                         - run clang-tidy on C++ files'
    @echo 'format                       - run clang-format on C++ files'
    @echo '-- HELP --'
    @echo 'help                         - display this help message'