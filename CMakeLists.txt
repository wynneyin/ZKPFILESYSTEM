cmake_minimum_required(VERSION 3.16)
project(zkpfs VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(zkpcrypto
    src/crypto/schnorr.cpp
    src/crypto/symcrypto.cpp)
target_link_libraries(zkpcrypto PUBLIC OpenSSL::Crypto)
target_compile_options(zkpcrypto PRIVATE -Wall -Wextra -Werror)

add_library(zkpfs_lib
    src/zkpfs/fileutil.cpp
    src/zkpfs/manifest.cpp
    src/zkpfs/vault.cpp)
target_link_libraries(zkpfs_lib PUBLIC zkpcrypto nlohmann_json::nlohmann_json)
target_compile_options(zkpfs_lib PRIVATE -Wall -Wextra -Werror)

add_executable(zkpfs src/zkpfs/main.cpp)
target_link_libraries(zkpfs PRIVATE zkpfs_lib)

add_executable(test_zkpfs tests/test_zkpfs.cpp)
target_link_libraries(test_zkpfs PRIVATE zkpfs_lib)

enable_testing()
add_test(NAME zkpfs_tests COMMAND test_zkpfs)
