include(ExternalProject)

#BlockCompressor source variables
set(CDIR ${CMAKE_CURRENT_SOURCE_DIR}/BlockCompressor)

#XZ source variables
set(XZ_INCLUDES ${CMAKE_BINARY_DIR}/xz/include)
set(XZ_LIB ${CMAKE_BINARY_DIR}/xz/lib/liblzma.a)
set(XZ_SOURCES ${XZ_INCLUDES}/lzma.h)

#ConfigurationLiterate source variables
set(CONFIGURATIONLITERATE_LIB ${CMAKE_BINARY_DIR}/plugins/libConfigurationLiterate.a)
set(CONFIGURATIONLITERATE_SOURCES ${CDIR}/ConfigurationLiterate.cpp ${CDIR}/ConfigurationLiterate.h)

#Block De/Compressor sources
set(BLOCKCOMPRESSOR_SOURCES ${CDIR}/BlockCompressor.cpp ${CDIR}/BlockCompressor.h)
set(BLOCKDECOMPRESSOR_SOURCES ${CDIR}/BlockDecompressor.cpp ${CDIR}/BlockDecompressor.h)


########### XZ Utils ###########

# Define the XZ Utils repository URL and tag
set(XZ_REPO_URL "https://git.tukaani.org/xz.git")
set(XZ_REPO_TAG "v5.4.6")  # Adjust the tag as needed

# Add XZ Utils as an external project
ExternalProject_Add(
    xz
    PREFIX xz
    GIT_REPOSITORY ${XZ_REPO_URL}
    GIT_TAG ${XZ_REPO_TAG}
    UPDATE_COMMAND ""
	
    CONFIGURE_COMMAND
    	cd ${CMAKE_BINARY_DIR}/plugins/xz/src/xz &&
        ${CMAKE_BINARY_DIR}/plugins/xz/src/xz/autogen.sh && 
        ./configure --prefix=${CMAKE_BINARY_DIR}/xz --disable-shared 

    INSTALL_COMMAND 
        #Compile only liblzma
        make -C ${CMAKE_BINARY_DIR}/plugins/xz/src/xz/src/liblzma install &&
        cd -
        
    BUILD_COMMAND ""
    
    BUILD_BYPRODUCTS ${XZ_LIB} ${XZ_SOURCES}
)


########### ConfigurationLiterate ###########
add_library(ConfigurationLiterate STATIC ${CONFIGURATIONLITERATE_SOURCES})
target_include_directories(ConfigurationLiterate PUBLIC "${CDIR}")
target_link_libraries(ConfigurationLiterate PRIVATE build_type_flags headers)

####### BlockCompressor (plugin lib) ########
add_library(BlockCompressor SHARED ${BLOCKCOMPRESSOR_SOURCES} ${CONFIGURATIONLITERATE_SOURCES} ${XZ_SOURCES})
add_dependencies(BlockCompressor xz ConfigurationLiterate)
target_link_libraries(BlockCompressor PRIVATE build_type_flags headers ${XZ_LIB} ${CONFIGURATIONLITERATE_LIB})
target_include_directories(BlockCompressor PUBLIC ${XZ_INCLUDES} ${CDIR})

###### mainBlockDecompressor (executable) #######
add_executable(BlockDecompressor ${CDIR}/mainBlockDecompressor.cpp ${BLOCKDECOMPRESSOR_SOURCES} ${CONFIGURATIONLITERATE_SOURCES} ${XZ_SOURCES})
add_dependencies(BlockDecompressor xz ConfigurationLiterate)
target_link_libraries(BlockDecompressor PRIVATE ${XZ_LIB} ${CONFIGURATIONLITERATE_LIB})
target_include_directories(BlockDecompressor PUBLIC ${XZ_INCLUDES} ${CDIR})

###### benchmark (executable) #######
add_executable(benchmark ${CDIR}/benchmark.cpp ${BLOCKCOMPRESSOR_SOURCES} ${BLOCKDECOMPRESSOR_SOURCES} ${CONFIGURATIONLITERATE_SOURCES} ${XZ_SOURCES})
add_dependencies(benchmark xz ConfigurationLiterate)
target_link_libraries(benchmark PRIVATE ${XZ_LIB} ${CONFIGURATIONLITERATE_LIB})
target_include_directories(benchmark PUBLIC ${XZ_INCLUDES} ${CDIR})

