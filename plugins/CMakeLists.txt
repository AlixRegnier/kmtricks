
file(GLOB_RECURSE PLUGIN_SOURCES "*.cpp")

add_custom_target(plugins)
foreach(PLUGIN ${PLUGIN_SOURCES})
  get_filename_component(PLUGIN_LIB ${PLUGIN} NAME_WLE)
  add_library(${PLUGIN_LIB} SHARED ${PLUGIN})
  target_link_libraries(${PLUGIN_LIB} PRIVATE build_type_flags headers)
  target_compile_definitions(${PLUGIN_LIB} PRIVATE DMAX_C=${MAX_C})
  add_dependencies(plugins ${PLUGIN_LIB})
endforeach()

include(ExternalProject)

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
  	  ./autogen.sh && 
	    ./configure --prefix=${CMAKE_BINARY_DIR}/xz --disable-shared && 
      make -C src/liblzma install &&
	    cd -

	  #make -C ${CMAKE_BINARY_DIR}/plugins/xz/src/xz/src/liblzma install &&
    INSTALL_COMMAND ""
    BUILD_COMMAND ""
)

# Add a dependency on the XZ library
add_dependencies(BlockCompressor xz)

# Specify include directories for XZ
target_include_directories(BlockCompressor PUBLIC ${CMAKE_BINARY_DIR}/xz/include)

# Specify link directories for XZ
target_link_directories(BlockCompressor PRIVATE ${CMAKE_BINARY_DIR}/xz/lib)

target_link_libraries(BlockCompressor PRIVATE build_type_flags headers)# ${CMAKE_BINARY_DIR}/xz/lib/liblzma.a)
