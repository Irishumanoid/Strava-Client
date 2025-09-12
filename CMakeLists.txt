cmake_minimum_required(VERSION 3.10.0)
project(Strava-analysis VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    src
    external
    external/nlohmann
    external/plog/include
)

find_package(OpenSSL REQUIRED)

add_executable(Strava-analysis
    src/main.cpp
    src/utils.cpp
    src/route_analysis/route_utils.cpp
)

target_link_libraries(Strava-analysis PRIVATE OpenSSL::SSL OpenSSL::Crypto)
