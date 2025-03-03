cmake_minimum_required(VERSION 3.12)
project(retardlauncher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
src/main.cpp
include/mojang/api/api.cpp
include/mojang/game/launch.cpp
include/mojang/auth/offline.cpp
include/utils/utils.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/include)

find_package(OpenSSL REQUIRED)
find_package(cpr REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

