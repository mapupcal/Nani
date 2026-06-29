find_package(GTest CONFIG REQUIRED)

nani_add_executable(canvas_utests
	main.cpp
	defs.h
	test_color.cpp
	test_elements.cpp
	test_geometry.cpp
	test_polygonf.cpp
	test_styles.cpp
	test_transformf.cpp
	test_window.cpp
	test_view.cpp
)

target_include_directories(canvas_utests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(canvas_utests PRIVATE cxx_std_20)

target_link_libraries(canvas_utests PRIVATE
	canvas
	GTest::gtest
)

set_target_properties(canvas_utests PROPERTIES
    FOLDER "utests"
)

# Register test
enable_testing()
add_test(NAME canvas_utests COMMAND canvas_utests)
