cmake_minimum_required(VERSION 3.5)
project( SymbolEnv VERSION 0.1.0 )

add_library(SymbolEnv STATIC src/SymbolEnv.c)

target_include_directories( SymbolEnv PUBLIC ${PROJECT_SOURCE_DIR}/include )
target_sources( SymbolEnv PRIVATE ${PROJECT_SOURCE_DIR}/src/SymbolEnv )
target_sources( SymbolEnv PUBLIC ${PROJECT_SOURCE_DIR}/include/SymbolEnv )

set_target_properties(SymbolEnv
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

if(NOT TARGET HashTable)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/HashTable ${CMAKE_SOURCE_DIR}/ext/HashTable/build/bin)
endif(NOT TARGET HashTable)
target_link_libraries(SymbolEnv HashTable)

if(NOT TARGET LinkedList)
	add_subdirectory(${CMAKE_SOURCE_DIR}/ext/LinkedList ${CMAKE_SOURCE_DIR}/ext/LinkedList/build/bin)
endif(NOT TARGET LinkedList)
target_link_libraries(SymbolEnv LinkedList)
