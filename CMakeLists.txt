cmake_minimum_required(VERSION 3.23)
project(alicia_launcher
    LANGUAGES CXX
    DESCRIPTION "Simple Alicia launcher")

add_subdirectory(3rd-party)

add_library(project_properties
        INTERFACE)
target_compile_features(project_properties
        INTERFACE cxx_std_20)

add_executable(alicia_launcher
        src/main.cpp)
target_link_libraries(alicia_launcher
        PUBLIC project_properties
        PRIVATE spdlog nlohmann_json)
