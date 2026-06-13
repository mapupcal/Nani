nani_add_executable(runtime_snippets
	main.cpp
)

target_include_directories(runtime_snippets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
target_compile_features(runtime_snippets PRIVATE cxx_std_20)
target_link_libraries(runtime_snippets PRIVATE
    runtime
)
set_target_properties(runtime_snippets PROPERTIES
    FOLDER "snippets"
)
