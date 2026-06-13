nani_add_executable(canvas_snippets
	main.cpp
)

target_include_directories(canvas_snippets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
target_compile_features(canvas_snippets PRIVATE cxx_std_20)
target_link_libraries(canvas_snippets PRIVATE
    canvas
)
set_target_properties(canvas_snippets PROPERTIES
    FOLDER "snippets"
)