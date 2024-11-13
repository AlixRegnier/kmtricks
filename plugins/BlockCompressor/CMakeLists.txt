cmake_minimum_required(VERSION 3.14)
project(BlockCompressor)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)


# Add the include directories
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/LZMA
    ${PROJECT_SOURCE_DIR}/ZSTD
    ${PROJECT_BINARY_DIR}/xz-utils/include
    ${PROJECT_BINARY_DIR}/zstd/include
    ${PROJECT_BINARY_DIR}/sdsl/include
)

# Download and build XZ Utils as an External Project
include(ExternalProject)
ExternalProject_Add(xz-utils

    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG v5.6.2

    CONFIGURE_COMMAND
        cd ${PROJECT_BINARY_DIR}/xz-utils-prefix/src/xz-utils/ &&
        ${PROJECT_BINARY_DIR}/xz-utils-prefix/src/xz-utils/autogen.sh && 
        ${PROJECT_BINARY_DIR}/xz-utils-prefix/src/xz-utils/configure --prefix=${PROJECT_BINARY_DIR}/xz-utils --disable-shared 

    INSTALL_COMMAND 
        #Compile only liblzma
        make -C ${PROJECT_BINARY_DIR}/xz-utils-prefix/src/xz-utils/src/liblzma install &&
        cd -
        
    BUILD_COMMAND ""
    
    #BUILD_BYPRODUCTS ${XZ_LIB} ${XZ_SOURCES}
)

# Download and build Zstd as an External Project
ExternalProject_Add(zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.5
    SOURCE_SUBDIR build/cmake
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/zstd
        -DZSTD_BUILD_STATIC=OFF
        -DZSTD_BUILD_SHARED=ON
        -DZSTD_BUILD_PROGRAMS=OFF
        -DZSTD_BUILD_TESTS=OFF
)

# Download and build SDSL as an External Project
ExternalProject_Add(sdsl
    GIT_REPOSITORY https://github.com/simongog/sdsl-lite.git
    GIT_TAG v2.1.1
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/sdsl -DBUILD_SHARED_LIBS=ON
)


# Build the ConfigurationLiterate library
add_library(ConfigurationLiterate SHARED
    ConfigurationLiterate.cpp
    ConfigurationLiterate.h
)

# Build the BlockCompressor library
add_library(BlockCompressor SHARED
    BlockCompressor.cpp
    BlockCompressor.h
)
target_link_libraries(BlockCompressor PRIVATE ConfigurationLiterate ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)
add_dependencies(BlockCompressor sdsl)

# Build the BlockDecompressor library 
add_library(BlockDecompressor STATIC
    BlockDecompressor.cpp
    BlockDecompressor.h
)
target_link_libraries(BlockDecompressor PRIVATE ConfigurationLiterate ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)
add_dependencies(BlockDecompressor sdsl)

# Build the BlockCompressorLZMA library
add_library(BlockCompressorLZMA SHARED
    LZMA/BlockCompressorLZMA.cpp
    LZMA/BlockCompressorLZMA.h
)
add_dependencies(BlockCompressorLZMA xz-utils)
target_link_libraries(BlockCompressorLZMA PUBLIC BlockCompressor ${PROJECT_BINARY_DIR}/xz-utils/lib/liblzma.a ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)

# Build the BlockDecompressorLZMA library
add_library(BlockDecompressorLZMA STATIC
    LZMA/BlockDecompressorLZMA.cpp
    LZMA/BlockDecompressorLZMA.h
)
add_dependencies(BlockDecompressorLZMA xz-utils)
target_link_libraries(BlockDecompressorLZMA PUBLIC BlockDecompressor ${PROJECT_BINARY_DIR}/xz-utils/lib/liblzma.a ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)

# Build the mainBlockCompressorLZMA executable
add_executable(mainBlockCompressorLZMA
    LZMA/mainBlockCompressorLZMA.cpp
)
target_link_libraries(mainBlockCompressorLZMA PUBLIC BlockCompressorLZMA)

# Build the mainBlockDecompressorLZMA executable
add_executable(mainBlockDecompressorLZMA
    LZMA/mainBlockDecompressorLZMA.cpp
)
target_link_libraries(mainBlockDecompressorLZMA PUBLIC BlockDecompressorLZMA)

####### ZSTD #######


# Build the BlockCompressorZSTD library
add_library(BlockCompressorZSTD SHARED
    ZSTD/BlockCompressorZSTD.cpp
    ZSTD/BlockCompressorZSTD.h
)
add_dependencies(BlockCompressorZSTD zstd)
target_link_libraries(BlockCompressorZSTD PUBLIC BlockCompressor ${PROJECT_BINARY_DIR}/zstd/lib/libzstd.so ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)

# Build the BlockDecompressorZSTD library
add_library(BlockDecompressorZSTD STATIC
    ZSTD/BlockDecompressorZSTD.cpp
    ZSTD/BlockDecompressorZSTD.h
)
add_dependencies(BlockDecompressorZSTD zstd)
target_link_libraries(BlockDecompressorZSTD PUBLIC BlockDecompressor ${PROJECT_BINARY_DIR}/zstd/lib/libzstd.so ${PROJECT_BINARY_DIR}/sdsl/lib/libsdsl.so)

# Build the mainBlockCompressorZSTD executable
add_executable(mainBlockCompressorZSTD
    ZSTD/mainBlockCompressorZSTD.cpp
)
target_link_libraries(mainBlockCompressorZSTD PUBLIC BlockCompressorZSTD)

# Build the mainBlockDecompressorZSTD executable
add_executable(mainBlockDecompressorZSTD
    ZSTD/mainBlockDecompressorZSTD.cpp
)
target_link_libraries(mainBlockDecompressorZSTD PUBLIC BlockDecompressorZSTD)

####### benchmark #######

# Build benchmarks
add_executable(benchmark
    benchmark.cpp
)
target_link_libraries(benchmark PUBLIC BlockCompressorLZMA BlockDecompressorLZMA BlockCompressorZSTD BlockDecompressorZSTD)


list(APPEND BLOCK_COMPRESSOR_DEP_KMTRICKS BlockCompressor BlockDecompressor BlockCompressorLZMA BlockDecompressorLZMA mainBlockCompressorLZMA mainBlockDecompressorLZMA BlockCompressorZSTD BlockDecompressorZSTD mainBlockCompressorZSTD mainBlockDecompressorZSTD benchmark)

foreach(BC_DEP ${BLOCK_COMPRESSOR_DEP_KMTRICKS})
  target_link_libraries(${BC_DEP} PRIVATE build_type_flags headers)
  target_compile_definitions(${BC_DEP} PRIVATE DMAX_C=${MAX_C})
  add_dependencies(plugins ${BC_DEP})
endforeach()