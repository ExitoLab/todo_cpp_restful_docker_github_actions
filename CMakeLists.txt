cmake_minimum_required(VERSION 3.10)
project(todo_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(libmongocxx REQUIRED)
find_package(libbsoncxx REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Add Crow include directories
include_directories(${LIBMONGOCXX_INCLUDE_DIRS})
include_directories(${LIBBSONCXX_INCLUDE_DIRS})
include_directories("/usr/local/include")

# Add executable
add_executable(todo_cpp main.cpp)

# Link libraries
target_link_libraries(todo_cpp
    PRIVATE
    ${LIBMONGOCXX_LIBRARIES}
    ${LIBBSONCXX_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Add compiler definitions
target_compile_definitions(todo_cpp
    PRIVATE
    ${LIBMONGOCXX_DEFINITIONS}
    ${LIBBSONCXX_DEFINITIONS}
)