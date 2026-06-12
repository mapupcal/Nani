find_package(nfd CONFIG REQUIRED)

nani_add_library(runtime SHARED
	defs.h
	directory.cpp
	directory.h
	file_dialog.cpp
	file_dialog.h
)
target_include_directories(runtime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(runtime PRIVATE NANI_RUNTIME_IMPLEMENTATION)
target_compile_features(runtime PRIVATE cxx_std_20)
target_link_libraries(runtime PRIVATE nfd::nfd)