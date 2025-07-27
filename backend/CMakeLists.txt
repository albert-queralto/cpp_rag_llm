cmake_minimum_required(VERSION 3.10)

# Set the project name
project(llm_backend)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/local/include)

# Add the executable
add_executable(llm_backend ${SOURCES})

# Link libraries
find_package(CURL REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(
    llm_backend 
    PRIVATE CURL::libcurl 
    Boost::boost 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    pthread 
    sqlite3 
    pqxx 
    pq 
    ssl 
    PRIVATE chromadb
    crypto)